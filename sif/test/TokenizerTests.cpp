/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-11-05
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"
#include "Fixtures.h"

#include <string>
#include <vector>

#include "sif/layout_engine/Parser.h"
#include "sif/layout_engine/Tokenizer.h"
#include "sif/layout_engine/UIFactory.h"
#include "sif/layout_engine/elements/Rectangle.h"

using namespace sif;

namespace {
    const std::string k_path = "sif_test_tokenizer.ui.xml";

    std::vector<ui::Token> tokenize(const std::string& xml) {
        sif::test::fixtures::write_file(k_path, xml);
        return ui::Tokenizer::tokenize(k_path);
    }

    /// @brief The tokenizer's error message for xml, or "" if it was accepted.
    std::string error_of(const std::string& xml) {
        try {
            (void)tokenize(xml);
        } catch (const std::exception& e) {
            return e.what();
        }
        return {};
    }

    /// @brief First tag token named name (open or self-closing), or nullptr.
    const ui::Token* tag(const std::vector<ui::Token>& tokens, const std::string& name) {
        for (const ui::Token& t : tokens) {
            if ((t.type == ui::TokenType::OpenTag || t.type == ui::TokenType::SelfCloseTag) && t.name == name) {
                return &t;
            }
        }
        return nullptr;
    }

    bool contains(const std::string& haystack, const std::string& needle) {
        return haystack.find(needle) != std::string::npos;
    }
}

// =================================================================
// Comments, prolog, malformed input (audit 1.2)
// =================================================================

SIF_TEST(tokenizer_skips_comments_prolog_and_doctype) {
    // Each of these used to hang the tokenizer forever.
    const auto tokens = tokenize(R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE ui>
<!-- a scene -->
<VBox name="root">
    <!-- a <Rectangle/> inside a comment is not a tag -->
    <Rectangle name="r"/>
</VBox>
)");
    SIF_CHECK(tag(tokens, "VBox") != nullptr);
    SIF_CHECK(tag(tokens, "Rectangle") != nullptr);

    std::size_t tags = 0;
    for (const ui::Token& t : tokens) {
        if (t.type != ui::TokenType::Text) {
            ++tags;
        }
    }
    SIF_CHECK(tags == 3); // <VBox>, <Rectangle/>, </VBox>
}

SIF_TEST(tokenizer_drops_a_comment_inside_text_instead_of_cutting_the_text) {
    const auto tokens = tokenize("<Text>Hello <!-- not shown -->world</Text>");
    const ui::Token* text = tag(tokens, "Text");
    SIF_CHECK(text != nullptr);
    if (text) {
        SIF_CHECK(text->attributes.at("inner_text") == "Hello world");
    }
}

SIF_TEST(a_scene_with_comments_builds_end_to_end) {
    sif::test::fixtures::write_file(k_path, R"(<!-- header -->
<Overlay name="root">
    <!-- background -->
    <Rectangle name="bg" fill_color="1|2|3"/>
</Overlay>
)");
    const auto node = ui::Parser::parse(ui::Tokenizer::tokenize(k_path));
    const auto root = ui::UIFactory::instance().build(*node);
    SIF_CHECK(root->find_by_name<ui::Rectangle>("bg") != nullptr);
}

SIF_TEST(malformed_markup_is_an_error_with_a_position_not_a_hang) {
    const std::string stray = error_of("<VBox>\n    <Rectangle name=\"r\" !/>\n</VBox>");
    SIF_CHECK(contains(stray, "unexpected character '!'"));
    SIF_CHECK(contains(stray, "line 2, column 25"));

    SIF_CHECK(contains(error_of("<VBox width=PX|10/>"), "unexpected character '|'"));
    SIF_CHECK(contains(error_of("<VBox><!-- never closed </VBox>"), "unterminated comment"));
    SIF_CHECK(contains(error_of("<VBox name=\"open"), "unterminated value of attribute 'name'"));
    SIF_CHECK(contains(error_of("<VBox name=\"x\""), "unterminated tag <VBox>"));
    SIF_CHECK(contains(error_of("<VBox></VBox"), "unterminated closing tag"));
    SIF_CHECK(contains(error_of("< >"), "expected a tag name"));
    SIF_CHECK(contains(error_of("<VBox/ >"), "expected '>' after '/'"));
    SIF_CHECK(contains(error_of("<Text><![CDATA[x]]></Text>"), "CDATA"));
    SIF_CHECK(contains(error_of("<?xml version=\"1.0\""), "unterminated processing instruction"));

    // Still accepted, exactly as before: a valueless attribute and
    // whitespace inside a closing tag.
    SIF_CHECK(error_of("<VBox flag></VBox >").empty());
}

// =================================================================
// Entities and escaping (audit 1.4)
// =================================================================

SIF_TEST(tokenizer_decodes_entities_in_attributes_and_text) {
    const auto tokens = tokenize(
        R"(<Text name="a &quot;quoted&quot; &amp; &lt;b&gt; &apos;x&apos;">5 &lt; 6 &#1055;&#x456; &#65;</Text>)");
    const ui::Token* text = tag(tokens, "Text");
    SIF_CHECK(text != nullptr);
    if (text) {
        SIF_CHECK(text->attributes.at("name") == "a \"quoted\" & <b> 'x'");
        SIF_CHECK(text->attributes.at("inner_text") == "5 < 6 \xD0\x9F\xD1\x96 A"); // "Пі" in UTF-8
    }
}

SIF_TEST(unknown_entities_and_bare_ampersands_are_kept_literally) {
    const auto tokens = tokenize(R"(<Text name="Tom & Jerry">&foo; &#xD800; &#; & done</Text>)");
    const ui::Token* text = tag(tokens, "Text");
    SIF_CHECK(text != nullptr);
    if (text) {
        SIF_CHECK(text->attributes.at("name") == "Tom & Jerry");
        SIF_CHECK(text->attributes.at("inner_text") == "&foo; &#xD800; &#; & done");
    }
}

SIF_TEST(save_tokens_escapes_so_values_survive_a_round_trip) {
    // The path a scene takes through Asset_Reference_Serialization:
    // tokenize, rewrite, save_tokens, and later tokenize again.
    const std::string tricky = "say \"hi\" & <wave> 'now'";

    std::vector<ui::Token> tokens;
    ui::Token open{ui::TokenType::OpenTag, "Text", {}};
    open.attributes.emplace("name", tricky);
    open.attributes.emplace("inner_text", tricky);
    tokens.push_back(open);
    tokens.push_back(ui::Token{ui::TokenType::Text, "a < b & c", {}});
    tokens.push_back(ui::Token{ui::TokenType::CloseTag, "Text", {}});

    ui::Tokenizer::save_tokens(k_path, tokens);
    const auto reread = ui::Tokenizer::tokenize(k_path);

    const ui::Token* text = tag(reread, "Text");
    SIF_CHECK(text != nullptr);
    if (text) {
        SIF_CHECK(text->attributes.at("name") == tricky);
        SIF_CHECK(text->attributes.at("inner_text") == tricky);
    }

    // And a second round trip changes nothing.
    ui::Tokenizer::save_tokens(k_path, reread);
    const auto again = ui::Tokenizer::tokenize(k_path);
    const ui::Token* text2 = tag(again, "Text");
    SIF_CHECK(text2 != nullptr && text2->attributes.at("name") == tricky);
}
