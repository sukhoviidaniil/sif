/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "sif/diagnostics/Logger.h"
#include "sif/layout_engine/Tokenizer.h"

namespace sif::ui {

    static void skip_ws(const std::string_view src, size_t& i) {
        while (i < src.size() && std::isspace(static_cast<unsigned char>(src[i]))) {
            ++i;
        }
    }

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

        while (i < src.size()) {
            // ---------- TEXT ----------
            if (src[i] != '<') {
                size_t start = i;
                while (i < src.size() && src[i] != '<') {
                    ++i;
                }

                std::string text(src.substr(start, i - start));
                if (!text.empty()) {
                    tokens.push_back(Token{TokenType::Text, std::move(text), {}});
                }
                continue;
            }

            // ---------- TAG ----------
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

            Token token;
            token.name = std::move(tagName);

            // ---------- CLOSE TAG ----------
            if (isClosing) {
                // skip until '>'
                while (i < src.size() && src[i] != '>') {
                    ++i;
                }
                if (i < src.size())
                    ++i;

                token.type = TokenType::CloseTag;
                tokens.push_back(std::move(token));
                continue;
            }

            // ---------- ATTRIBUTES ----------
            while (i < src.size()) {
                skip_ws(src, i);

                if (src[i] == '/' || src[i] == '>') {
                    break;
                }

                // attribute name
                size_t attrNameStart = i;
                while (i < src.size() && (std::isalnum(static_cast<unsigned char>(src[i])) || src[i] == '_')) {
                    ++i;
                }

                std::string attrName(src.substr(attrNameStart, i - attrNameStart));
                skip_ws(src, i);

                if (i < src.size() && src[i] == '=') {
                    ++i;
                    skip_ws(src, i);
                }

                // attribute value
                std::string attrValue;
                if (i < src.size() && src[i] == '"') {
                    ++i;
                    size_t valueStart = i;
                    while (i < src.size() && src[i] != '"') {
                        ++i;
                    }
                    attrValue = std::string(src.substr(valueStart, i - valueStart));
                    if (i < src.size())
                        ++i;
                }

                token.attributes.emplace(std::move(attrName), std::move(attrValue));
            }

            // ---------- TAG END ----------
            if (i < src.size() && src[i] == '/') {
                ++i; // skip '/'
                if (i < src.size() && src[i] == '>')
                    ++i;
                token.type = TokenType::SelfCloseTag;
            } else {
                if (i < src.size() && src[i] == '>')
                    ++i;
                token.type = TokenType::OpenTag;
                // If it is <Text>, parse the content into the “text” attribute
                if (token.name == "Text") {
                    size_t textStart = i;
                    while (i < src.size() && src[i] != '<') {
                        ++i;
                    }
                    std::string text_content = src.substr(textStart, i - textStart);

                    // Normalization: removing indents and hyphenations
                    size_t first = text_content.find_first_not_of(" \t\n\r");
                    size_t last = text_content.find_last_not_of(" \t\n\r");
                    if (first != std::string::npos && last != std::string::npos) {
                        text_content = text_content.substr(first, last - first + 1);
                    } else {
                        text_content.clear();
                    }

                    token.attributes.emplace("inner_text", std::move(text_content));

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
                file << token.name; // для текста поле name содержит сам текст
                break;

            case TokenType::OpenTag:
                file << "<" << token.name;
                for (const auto& [attrName, attrValue] : token.attributes) {
                    file << " " << attrName << "=\"" << attrValue << "\"";
                }
                file << ">";
                break;

            case TokenType::CloseTag:
                file << "</" << token.name << ">";
                break;

            case TokenType::SelfCloseTag:
                file << "<" << token.name;
                for (const auto& [attrName, attrValue] : token.attributes) {
                    file << " " << attrName << "=\"" << attrValue << "\"";
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
