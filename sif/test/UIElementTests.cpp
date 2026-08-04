/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"
#include "Fixtures.h"

#include "sif/layout_engine/Tokenizer.h"
#include "sif/layout_engine/Parser.h"
#include "sif/layout_engine/UIFactory.h"
#include "sif/layout_engine/elements/Button.h"
#include "sif/layout_engine/elements/Menu.h"
#include "sif/layout_engine/elements/Rectangle.h"

using namespace sif;

namespace {
    /**
     * @brief Sample UI markup used only by the tests below.
     *
     * Kept as data (a string constant), separate from the assertions
     * that exercise it, so a future test needing a differently shaped
     * tree can add its own sample without touching this one.
     */
    constexpr const char* k_sample_ui_xml = R"(
<Element width="PERCENT|100" height="PERCENT|100">
    <Menu name="main_menu" spacing="4">
        <Button id="1" name="btn_play" width="PX|100" height="PX|30"/>
        <Button id="2" name="btn_quit" width="PX|100" height="PX|30"/>
    </Menu>
    <Rectangle name="bg" fill_color="10|10|10"/>
</Element>
)";

    std::unique_ptr<ui::UIElement> build_sample_tree() {
        const std::string path = "sif_test_sample.ui.xml";
        sif::test::fixtures::write_file(path, k_sample_ui_xml);

        const auto tokens = ui::Tokenizer::tokenize(path);
        const auto node = ui::Parser::parse(tokens);
        return ui::UIFactory::instance().build(*node);
    }
}

SIF_TEST(ui_factory_builds_a_tree_from_xml) {
    const auto root = build_sample_tree();
    SIF_CHECK(root != nullptr);
    SIF_CHECK(!root->children.empty());
}

SIF_TEST(find_by_name_and_find_by_id_preserve_type) {
    const auto root = build_sample_tree();

    ui::Menu* menu = root->find_by_name<ui::Menu>("main_menu");
    SIF_CHECK(menu != nullptr);

    ui::Button* play_btn = root->find_by_name<ui::Button>("btn_play");
    SIF_CHECK(play_btn != nullptr);

    ui::Button* by_id = root->find_by_id<ui::Button>(intrnl::RecordID(1));
    SIF_CHECK(by_id == play_btn);

    ui::Rectangle* rect = root->find_by_name<ui::Rectangle>("bg");
    SIF_CHECK(rect != nullptr);

    // A name that exists but names the wrong concrete type must yield
    // nullptr, not a silently-wrong pointer: this is the "preserving
    // typing" requirement, checked via a deliberate type mismatch.
    SIF_CHECK(root->find_by_name<ui::Button>("bg") == nullptr);

    SIF_CHECK(root->find_by_name<ui::UIElement>("does_not_exist") == nullptr);
}

SIF_TEST(menu_tracks_xml_parsed_buttons) {
    const auto root = build_sample_tree();
    ui::Menu* menu = root->find_by_name<ui::Menu>("main_menu");
    SIF_CHECK(menu != nullptr);
    if (menu) {
        SIF_CHECK(menu->button_ids().size() == 2);
    }
}

SIF_TEST(menu_set_active_enforces_single_selection) {
    const auto root = build_sample_tree();
    ui::Menu* menu = root->find_by_name<ui::Menu>("main_menu");
    ui::Button* play_btn = root->find_by_name<ui::Button>("btn_play");
    ui::Button* quit_btn = root->find_by_name<ui::Button>("btn_quit");
    if (!menu || !play_btn || !quit_btn) { SIF_CHECK(false); return; }

    menu->set_active(intrnl::RecordID(2));
    SIF_CHECK(quit_btn->is_selected());
    SIF_CHECK(!play_btn->is_selected());
    SIF_CHECK(menu->active_button_id().has_value());
    SIF_CHECK(*menu->active_button_id() == intrnl::RecordID(2));

    menu->set_active(intrnl::RecordID(1));
    SIF_CHECK(play_btn->is_selected());
    SIF_CHECK(!quit_btn->is_selected());
}

SIF_TEST(menu_set_active_with_unknown_id_is_a_no_op) {
    const auto root = build_sample_tree();
    ui::Menu* menu = root->find_by_name<ui::Menu>("main_menu");
    if (!menu) { SIF_CHECK(false); return; }

    menu->set_active(intrnl::RecordID(1));
    menu->set_active(intrnl::RecordID(999)); // does not exist

    SIF_CHECK(menu->active_button_id().has_value());
    SIF_CHECK(*menu->active_button_id() == intrnl::RecordID(1)); // unchanged
}

SIF_TEST(button_contains_and_menu_handle_click_hit_test_correctly) {
    const auto root = build_sample_tree();
    ui::Menu* menu = root->find_by_name<ui::Menu>("main_menu");
    ui::Button* play_btn = root->find_by_name<ui::Button>("btn_play");
    if (!menu || !play_btn) { SIF_CHECK(false); return; }

    root->measure({800.f, 600.f});
    root->layout({0.f, 0.f, 800.f, 600.f});

    const intrnl::Rect r = play_btn->result.rect;
    const math::Vector2 inside{r.x + 1.f, r.y + 1.f};
    const math::Vector2 outside{r.x + r.width + 500.f, r.y + r.height + 500.f};

    SIF_CHECK(play_btn->contains(inside));
    SIF_CHECK(!play_btn->contains(outside));

    SIF_CHECK(menu->handle_click(inside));
    SIF_CHECK(play_btn->is_selected());
    SIF_CHECK(!menu->handle_click(outside));
}

SIF_TEST(button_update_advances_smooth_transition_without_throwing) {
    ui::Button button;
    button.select();
    for (int i = 0; i < 50; ++i) {
        button.update(0.01f);
    }
    SIF_CHECK(button.is_selected());
}
