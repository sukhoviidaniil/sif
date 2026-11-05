/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include <charconv>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "sif/diagnostics/Logger.h"
#include "sif/layout_engine/Tokenizer.h"

namespace sif::ui {

static void skip_ws(const std::string_view src, size_t& i) {
    while (i < src.size() && std::isspace(static_cast<unsigned char>(src[i]))) {
        ++i;
    }
}

namespace {
/// @brief True if src contains what starting exactly at offset i.
bool starts_with_at(const std::string_view src, const size_t i, const std::string_view what) {
    return i <= src.size() && src.substr(i).starts_with(what);
}

/**
 * @brief "line L, column C" (both 1-based) of byte offset i.
 *
 * Computed only when an error is reported, so a well-formed scene
 * pays nothing for it.
 */
std::string position_of(const std::string_view src, const size_t i) {
    size_t line = 1;
    size_t column = 1;
    for (size_t k = 0; k < i && k < src.size(); ++k) {
        if (src[k] == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }
    return "line " + std::to_string(line) + ", column " + std::to_string(column);
}

/// @brief Appends code point cp to out, encoded as UTF-8.
void append_utf8(std::string& out, const std::uint32_t cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

/**
 * @brief Decodes the five predefined XML entities and numeric
 * character references (&#1055; / &#x41;).
 *
 * Deliberately lenient: a bare '&' or an unknown entity such as
 * "&foo;" is kept literally instead of rejecting the scene, so text
 * like "Tom & Jerry" that worked before entities were understood
 * keeps working.
 */
std::string decode_entities(const std::string_view in) {
    std::string out;
    out.reserve(in.size());

    size_t i = 0;
    while (i < in.size()) {
        if (in[i] != '&') {
            out.push_back(in[i++]);
            continue;
        }

        const size_t semi = in.find(';', i + 1);
        // Longest reference worth decoding is "&#x10FFFF;" (10 chars).
        if (semi == std::string_view::npos || semi - i > 10) {
            out.push_back(in[i++]);
            continue;
        }

        const std::string_view name = in.substr(i + 1, semi - i - 1);
        if (name == "lt") {
            out.push_back('<');
        } else if (name == "gt") {
            out.push_back('>');
        } else if (name == "amp") {
            out.push_back('&');
        } else if (name == "quot") {
            out.push_back('"');
        } else if (name == "apos") {
            out.push_back('\'');
        } else if (name.size() > 1 && name[0] == '#') {
            const bool hex = name[1] == 'x' || name[1] == 'X';
            const std::string_view digits = name.substr(hex ? 2 : 1);
            std::uint32_t cp = 0;
            const auto [ptr, ec] = std::from_chars(digits.data(), digits.data() + digits.size(), cp, hex ? 16 : 10);
            const bool valid = !digits.empty() && ec == std::errc{} && ptr == digits.data() + digits.size() && cp > 0 &&
                               cp <= 0x10FFFF && (cp < 0xD800 || cp > 0xDFFF);
            if (!valid) {
                out.push_back(in[i++]); // not a usable reference: keep it literally
                continue;
            }
            append_utf8(out, cp);
        } else {
            out.push_back(in[i++]); // unknown entity: keep it literally
            continue;
        }
        i = semi + 1;
    }
    return out;
}

/**
 * @brief Escapes text for writing back into a *.ui.xml file.
 *
 * The inverse of decode_entities. Without it, a value containing a
 * '"' was written unquoted-safe and cut short at that quote the next
 * time the file was read (e.g. by the serialized scene the runtime
 * loads after Asset_Reference_Serialization).
 *
 * @param in Decoded text.
 * @param in_attribute true for an attribute value, which also needs
 * '"' escaped.
 */
std::string escape_xml(const std::string_view in, const bool in_attribute) {
    std::string out;
    out.reserve(in.size());
    for (const char c : in) {
        switch (c) {
        case '&':
            out += "&amp;";
            break;
        case '<':
            out += "&lt;";
            break;
        case '>':
            out += "&gt;";
            break;
        case '"':
            if (in_attribute) {
                out += "&quot;";
                break;
            }
            out.push_back(c);
            break;
        default:
            out.push_back(c);
            break;
        }
    }
    return out;
}
} // namespace

bool Tokenizer::is_ui_xml_file(const std::string& path) {
    try {
        ensure_ui_xml_file(path, true);
    } catch (const std::exception& e) {
        return false;
    }
    return true;
}

std::vector<Token> Tokenizer::tokenize(const std::string& path) {

    try {
        ensure_ui_xml_file(path, true);
    } catch (const std::exception& e) {
        std::string err = "Tokenizer::tokenize failed for file '" + path + "': " + e.what();
        LOG(err);
        throw std::runtime_error(err);
    }

    // load_file
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file");
    }

    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();
    if (size < 0) {
        throw std::runtime_error("Cannot determine the size of file: " + path);
    }
    std::string src(static_cast<std::size_t>(size), '\0');
    file.seekg(0);

    file.read(src.data(), size);
    //

    std::vector<Token> tokens;
    size_t i = 0;
    const std::string_view view(src);

    // Every malformed-input path ends here, with a position, instead of
    // looping forever: the attribute loop used to stop advancing on any
    // character it did not expect (an XML comment was enough), so a
    // single typo hung whatever loaded the scene.
    const auto fail = [&](const size_t at, const std::string& what) {
        const std::string err = "Tokenizer::tokenize - '" + path + "', " + position_of(view, at) + ": " + what;
        LOG(err);
        throw std::runtime_error(err);
    };

    // Line/column of each tag, tracked incrementally: tags are visited
    // in increasing order, so the whole file is scanned once rather
    // than once per tag (position_of is only for error messages).
    size_t pos_cursor = 0;
    size_t pos_line = 1;
    size_t pos_column = 1;
    const auto stamp = [&](Token& token, const size_t at) {
        for (; pos_cursor < at && pos_cursor < view.size(); ++pos_cursor) {
            if (view[pos_cursor] == '\n') {
                ++pos_line;
                pos_column = 1;
            } else {
                ++pos_column;
            }
        }
        token.line = pos_line;
        token.column = pos_column;
    };

    // Skips a comment, a processing instruction (<?xml ... ?>) or a
    // declaration (<!DOCTYPE ...>) starting at at; returns false if
    // there is none there.
    const auto skip_markup = [&](size_t& at) {
        if (starts_with_at(view, at, "<!--")) {
            const size_t end = view.find("-->", at + 4);
            if (end == std::string_view::npos) {
                fail(at, "unterminated comment");
            }
            at = end + 3;
            return true;
        }
        if (starts_with_at(view, at, "<![CDATA[")) {
            fail(at, "CDATA sections are not supported (use &lt; &gt; &amp; instead)");
        }
        if (starts_with_at(view, at, "<?")) {
            const size_t end = view.find("?>", at + 2);
            if (end == std::string_view::npos) {
                fail(at, "unterminated processing instruction");
            }
            at = end + 2;
            return true;
        }
        if (starts_with_at(view, at, "<!")) {
            const size_t end = view.find('>', at + 2);
            if (end == std::string_view::npos) {
                fail(at, "unterminated declaration");
            }
            at = end + 1;
            return true;
        }
        return false;
    };

    while (i < src.size()) {
        // ---------- TEXT ----------
        if (src[i] != '<') {
            size_t start = i;
            while (i < src.size() && src[i] != '<') {
                ++i;
            }

            std::string text = decode_entities(view.substr(start, i - start));
            if (!text.empty()) {
                tokens.push_back(Token{TokenType::Text, std::move(text), {}});
            }
            continue;
        }

        // ---------- COMMENT / PROLOG / DECLARATION ----------
        if (skip_markup(i)) {
            continue;
        }

        // ---------- TAG ----------
        const size_t tag_start = i;
        ++i; // skip '<'
        skip_ws(src, i);

        bool isClosing = false;
        if (i < src.size() && src[i] == '/') {
            isClosing = true;
            ++i;
            skip_ws(src, i);
        }

        // tag name
        size_t nameStart = i;
        while (i < src.size() && (std::isalnum(static_cast<unsigned char>(src[i])) || src[i] == '_')) {
            ++i;
        }

        std::string tagName(src.substr(nameStart, i - nameStart));
        if (tagName.empty()) {
            fail(tag_start, "expected a tag name after '<'");
        }

        Token token;
        token.name = std::move(tagName);
        stamp(token, tag_start);

        // ---------- CLOSE TAG ----------
        if (isClosing) {
            // skip until '>'
            while (i < src.size() && src[i] != '>') {
                ++i;
            }
            if (i >= src.size()) {
                fail(tag_start, "unterminated closing tag </" + token.name + ">");
            }
            ++i;

            token.type = TokenType::CloseTag;
            tokens.push_back(std::move(token));
            continue;
        }

        // ---------- ATTRIBUTES ----------
        while (true) {
            skip_ws(src, i);

            if (i >= src.size()) {
                fail(tag_start, "unterminated tag <" + token.name + ">");
            }
            if (src[i] == '/' || src[i] == '>') {
                break;
            }

            // attribute name
            size_t attrNameStart = i;
            while (i < src.size() && (std::isalnum(static_cast<unsigned char>(src[i])) || src[i] == '_')) {
                ++i;
            }

            std::string attrName(src.substr(attrNameStart, i - attrNameStart));
            if (attrName.empty()) {
                fail(i, std::string("unexpected character '") + src[i] + "' in <" + token.name + ">");
            }
            skip_ws(src, i);

            if (i < src.size() && src[i] == '=') {
                ++i;
                skip_ws(src, i);
            }

            // attribute value
            std::string attrValue;
            if (i < src.size() && src[i] == '"') {
                const size_t quote = i;
                ++i;
                size_t valueStart = i;
                while (i < src.size() && src[i] != '"') {
                    ++i;
                }
                if (i >= src.size()) {
                    fail(quote, "unterminated value of attribute '" + attrName + "'");
                }
                attrValue = decode_entities(view.substr(valueStart, i - valueStart));
                ++i;
            }

            token.attributes.emplace(std::move(attrName), std::move(attrValue));
        }

        // ---------- TAG END ----------
        if (src[i] == '/') {
            ++i; // skip '/'
            if (i >= src.size() || src[i] != '>') {
                fail(i, "expected '>' after '/' in <" + token.name + ">");
            }
            ++i;
            token.type = TokenType::SelfCloseTag;
        } else {
            ++i; // skip '>'
            token.type = TokenType::OpenTag;
            // If it is <Text>, parse the content into the “text” attribute
            if (token.name == "Text") {
                // Collected piece by piece so a comment inside the
                // text is dropped instead of ending it.
                std::string text_content;
                while (i < src.size()) {
                    if (src[i] == '<') {
                        if (starts_with_at(view, i, "<!--")) {
                            skip_markup(i);
                            continue;
                        }
                        break;
                    }
                    text_content.push_back(src[i++]);
                }

                // Normalization: removing indents and hyphenations
                size_t first = text_content.find_first_not_of(" \t\n\r");
                size_t last = text_content.find_last_not_of(" \t\n\r");
                if (first != std::string::npos && last != std::string::npos) {
                    text_content = text_content.substr(first, last - first + 1);
                } else {
                    text_content.clear();
                }

                // Decoded after trimming, so &#32; at either end survives.
                token.attributes.emplace("inner_text", decode_entities(text_content));

                // now i points to the closing tag '<'
            }
        }

        tokens.push_back(std::move(token));
    }

    return tokens;
}

void Tokenizer::save_tokens(const std::string& path, const std::vector<Token>& tokens) {

    try {
        ensure_ui_xml_file(path, false);
    } catch (const std::exception& e) {
        std::string err = "Tokenizer::save_tokens failed for file '" + path + "': " + e.what();
        LOG(err);
        throw std::runtime_error(err);
    }

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for writing");
    }

    for (const auto& token : tokens) {
        switch (token.type) {
        case TokenType::Text:
            // for text, the field name contains the text itself
            file << escape_xml(token.name, false);
            break;

        case TokenType::OpenTag:
            file << "<" << token.name;
            for (const auto& [attrName, attrValue] : token.attributes) {
                file << " " << attrName << "=\"" << escape_xml(attrValue, true) << "\"";
            }
            file << ">";
            break;

        case TokenType::CloseTag:
            file << "</" << token.name << ">";
            break;

        case TokenType::SelfCloseTag:
            file << "<" << token.name;
            for (const auto& [attrName, attrValue] : token.attributes) {
                file << " " << attrName << "=\"" << escape_xml(attrValue, true) << "\"";
            }
            file << "/>";
            break;
        }
    }

    file.close();
}

void Tokenizer::ensure_ui_xml_file(const std::string& path, bool must_exist) {
    std::filesystem::path p(path);

    if (must_exist) {
        if (!std::filesystem::exists(p) || !std::filesystem::is_regular_file(p)) {
            throw std::runtime_error("File does not exist or is not a regular file: " + path);
        }
    }

    std::string ext = p.extension().string();             // latest extension
    std::string stem_ext = p.stem().extension().string(); // extension before the last one

    if (ext != ".xml" || stem_ext != ".ui") {
        throw std::runtime_error("File must have '.ui.xml' extension: " + path);
    }
}
} // namespace sif::ui
