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

#include "sif/layout_engine/Parser.h"
#include "sif/layout_engine/Tokenizer.h"
#include "sif/layout_engine/UIFactory.h"
#include "sif/layout_engine/elements/Rectangle.h"

using namespace sif;

namespace {
    const std::string k_path = "sif_test_validation.ui.xml";

    std::unique_ptr<ui::UIElement> build_xml(const std::string& xml) {
        sif::test::fixtures::write_file(k_path, xml);
        const auto node = ui::Parser::parse(ui::Tokenizer::tokenize(k_path));
        return ui::UIFactory::instance().build(*node);
    }

    /// @brief The SceneError message for xml, or "" if it built.
    std::string scene_error(const std::string& xml) {
        try {
            (void)build_xml(xml);
        } catch (const ui::SceneError& e) {
            return e.what();
        }
        return {};
    }

    /**
     * @brief Turns strict mode on for one scope.
     *
     * UIFactory is a singleton shared by every test; leaving it strict
     * would change what later tests see.
     */
    struct StrictScope {
        StrictScope() { ui::UIFactory::instance().set_strict(true); }
        ~StrictScope() { ui::UIFactory::instance().set_strict(false); }
    };

    bool contains(const std::string& haystack, const std::string& needle) {
        return haystack.find(needle) != std::string::npos;
    }

    std::size_t occurrences(const std::string& haystack, const std::string& needle) {
        std::size_t n = 0;
        for (std::size_t at = haystack.find(needle); at != std::string::npos; at = haystack.find(needle, at + 1)) {
            ++n;
        }
        return n;
    }

    constexpr const char* k_typo_xml = "<Overlay>\n    <Rectangle name=\"bg\" fill_colour=\"1|2|3\"/>\n</Overlay>";
}

SIF_TEST(tokens_and_nodes_carry_the_position_of_their_tag) {
    sif::test::fixtures::write_file(k_path, "<Overlay>\n  <Rectangle/>\n    <Rectangle/>\n</Overlay>");
    const auto tokens = ui::Tokenizer::tokenize(k_path);
    SIF_CHECK(tokens.front().line == 1 && tokens.front().column == 1);

    const auto node = ui::Parser::parse(tokens);
    SIF_CHECK(node->children.size() == 2);
    SIF_CHECK(node->children[0]->line == 2 && node->children[0]->column == 3);
    SIF_CHECK(node->children[1]->line == 3 && node->children[1]->column == 5);
}

SIF_TEST(an_unknown_attribute_is_a_warning_with_position_and_suggestion) {
    const auto root = build_xml(k_typo_xml);

    // Still loads, exactly as before schemas existed...
    SIF_CHECK(root->find_by_name<ui::Rectangle>("bg") != nullptr);

    // ...but no longer silently.
    const auto& warnings = ui::UIFactory::instance().warnings();
    SIF_CHECK(warnings.size() == 1);
    if (warnings.size() == 1) {
        SIF_CHECK(contains(warnings[0], "line 2, column 5"));
        SIF_CHECK(contains(warnings[0], "<Rectangle> has no attribute 'fill_colour'"));
        SIF_CHECK(contains(warnings[0], "did you mean 'fill_color'?"));
    }

    // warnings() describes the most recent build only.
    (void)build_xml("<Overlay/>");
    SIF_CHECK(ui::UIFactory::instance().warnings().empty());
}

SIF_TEST(strict_mode_turns_an_unknown_attribute_into_an_error) {
    const StrictScope strict;
    const std::string error = scene_error(k_typo_xml);
    SIF_CHECK(contains(error, "line 2, column 5"));
    SIF_CHECK(contains(error, "fill_colour"));

    // Common attributes are always known, on every tag.
    SIF_CHECK(scene_error(R"(<Overlay name="a" id="1" visible="true" z_index="1" layer="Popup" hit_testable="false"
        margin="1" h_align="End" v_align="Center" offset="1|2"><Rectangle fill_color="1|2|3"/></Overlay>)")
                  .empty());
}

SIF_TEST(build_errors_point_at_the_element_once) {
    const std::string bad_value = scene_error("<Overlay>\n  <Overlay>\n    <Rectangle fill_color=\"red\"/>\n  </Overlay>\n</Overlay>");
    SIF_CHECK(contains(bad_value, "line 3, column 5: "));
    SIF_CHECK(contains(bad_value, "invalid fill_color"));
    SIF_CHECK(occurrences(bad_value, "line ") == 1); // not re-prefixed by every ancestor

    SIF_CHECK(contains(scene_error("<Overlay>\n  <Rectagle/>\n</Overlay>"), "line 2, column 3: Unknown tag: Rectagle"));
}

SIF_TEST(a_tag_registered_without_a_schema_accepts_any_attribute) {
    ui::UIFactory::instance().register_tag("SchemaFreeTestTag",
                                           [](const ui::Node&) { return std::make_unique<ui::UIElement>(); });
    const StrictScope strict;
    SIF_CHECK(scene_error(R"(<SchemaFreeTestTag anything="goes"/>)").empty());
}
