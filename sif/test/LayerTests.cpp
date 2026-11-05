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

#include <memory>
#include <string>
#include <vector>

#include "sif/layout_engine/Parser.h"
#include "sif/layout_engine/Tokenizer.h"
#include "sif/layout_engine/UIFactory.h"
#include "sif/layout_engine/elements/Button.h"
#include "sif/layout_engine/elements/Menu.h"
#include "sif/layout_engine/elements/Overlay.h"
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/render/FrameContext.h"
#include "sif/render/Layer.h"
#include "sif/render/RenderFrame.h"
#include "sif/render/elements/Rectangle.h"

using namespace sif;

namespace {
    /**
     * @brief A fixed-size, solid Rectangle whose fill colour's red
     * channel doubles as a tag, so a test can read the paint order back
     * out of a RenderFrame as a plain list of numbers.
     */
    std::unique_ptr<ui::Rectangle> tagged_rect(const std::uint8_t tag, const float w = 20.f, const float h = 10.f) {
        auto r = std::make_unique<ui::Rectangle>(intrnl::Color(tag, 0, 0));
        r->width = intrnl::Size::px(w);
        r->height = intrnl::Size::px(h);
        return r;
    }

    /// @brief Adds child to parent and returns a non-owning pointer to it.
    template<typename T>
    T* adopt(ui::UIElement& parent, std::unique_ptr<T> child) {
        T* raw = child.get();
        parent.add(std::move(child));
        return raw;
    }

    /// @brief Lays the tree out in rect.
    void lay_out(ui::UIElement& root, const intrnl::Rect rect) {
        root.measure({rect.width, rect.height});
        root.layout(rect);
    }

    /// @brief Renders root through the public entry point (UIElement::render).
    rnd::RenderFrame render_tree(const ui::UIElement& root) {
        rnd::RenderFrame frame;
        const rnd::FrameContext ctx(true);
        root.render(frame, ctx);
        return frame;
    }

    /// @brief The tags (see tagged_rect) of every rectangle, in draw order.
    std::vector<int> drawn_tags(const rnd::RenderFrame& frame) {
        std::vector<int> tags;
        for (const rnd::RenderNode* item : frame.draw_order()) {
            if (const auto* rect = dynamic_cast<const rnd::Rectangle*>(item)) {
                tags.push_back(rect->color.r);
            }
        }
        return tags;
    }

    std::unique_ptr<rnd::Rectangle> render_rect(const std::uint8_t tag, const int layer) {
        auto item = std::make_unique<rnd::Rectangle>();
        item->color = intrnl::Color(tag, 0, 0);
        item->layer = layer;
        return item;
    }

    /// @brief Tokenizes, parses and builds markup through a temporary scene file.
    std::unique_ptr<ui::UIElement> build_xml(const std::string& xml) {
        const std::string path = "sif_test_layers.ui.xml";
        sif::test::fixtures::write_file(path, xml);
        const auto tokens = ui::Tokenizer::tokenize(path);
        const auto node = ui::Parser::parse(tokens);
        return ui::UIFactory::instance().build(*node);
    }

    /// @brief True if building markup throws (i.e. the scene is rejected).
    bool rejects(const std::string& xml) {
        try {
            (void)build_xml(xml);
        } catch (const std::exception&) {
            return true;
        }
        return false;
    }

    /**
     * @brief Sample markup exercising every layering/overlap attribute.
     *
     * Kept as data, separate from the assertions, like the sample in
     * UIElementTests.cpp.
     */
    constexpr const char* k_layers_ui_xml = R"(
<Overlay name="root" width="PX|200" height="PX|100" padding="10">
    <Rectangle name="bg" h_align="Stretch" v_align="Stretch" fill_color="1|0|0"/>
    <Rectangle name="badge" width="PX|20" height="PX|10" h_align="End" v_align="Start"
               margin="5" offset="3|-4" z_index="2" fill_color="2|0|0"/>
    <Rectangle name="dialog" layer="Modal" visible="false" hit_testable="false" fill_color="3|0|0"/>
    <Rectangle name="tip" layer="250" margin="1|2" fill_color="4|0|0"/>
</Overlay>
)";
}

// =================================================================
// rnd: RenderFrame / LayerScope / named layers
// =================================================================

SIF_TEST(draw_order_without_layers_is_constant_items_then_temp_items) {
    rnd::RenderFrame frame;
    frame.temp_items.push_back(render_rect(3, 0));
    frame.constant_items.push_back(render_rect(1, 0));
    frame.temp_items.push_back(render_rect(4, 0));
    frame.constant_items.push_back(render_rect(2, 0));

    SIF_CHECK((drawn_tags(frame) == std::vector<int>{1, 2, 3, 4}));
}

SIF_TEST(draw_order_sorts_by_layer_and_keeps_submission_order_within_a_layer) {
    rnd::RenderFrame frame;
    frame.temp_items.push_back(render_rect(10, rnd::layer::Modal));
    frame.temp_items.push_back(render_rect(11, rnd::layer::Content));
    frame.constant_items.push_back(render_rect(12, rnd::layer::Content));
    frame.temp_items.push_back(render_rect(13, rnd::layer::Background));
    frame.temp_items.push_back(render_rect(14, rnd::layer::Modal));
    frame.temp_items.push_back(render_rect(15, rnd::layer::Content));

    // Background, then Content (constant bucket first, then temp in
    // submission order), then Modal in submission order.
    SIF_CHECK((drawn_tags(frame) == std::vector<int>{13, 12, 11, 15, 10, 14}));
}

SIF_TEST(submit_stamps_the_current_layer_and_layer_scopes_nest_and_restore) {
    rnd::RenderFrame frame;
    SIF_CHECK(frame.current_layer() == rnd::layer::Content);

    frame.submit(render_rect(1, 999)); // whatever the item carried is overwritten
    {
        const rnd::LayerScope outer(frame, rnd::layer::Modal);
        frame.submit(render_rect(2, 0));
        {
            const rnd::LayerScope inner(frame, rnd::layer::Tooltip);
            SIF_CHECK(frame.current_layer() == rnd::layer::Tooltip);
            frame.submit(render_rect(3, 0));
        }
        SIF_CHECK(frame.current_layer() == rnd::layer::Modal);
        frame.submit(render_rect(4, 0));
    }
    SIF_CHECK(frame.current_layer() == rnd::layer::Content);
    frame.submit(nullptr); // ignored rather than crashing draw_order later

    SIF_CHECK(frame.temp_items.size() == 4);
    SIF_CHECK(frame.temp_items[0]->layer == rnd::layer::Content);
    SIF_CHECK(frame.temp_items[1]->layer == rnd::layer::Modal);
    SIF_CHECK(frame.temp_items[2]->layer == rnd::layer::Tooltip);
    SIF_CHECK(frame.temp_items[3]->layer == rnd::layer::Modal);
}

SIF_TEST(layer_from_string_accepts_names_and_integers_only) {
    SIF_CHECK(rnd::layer_from_string("Background") == rnd::layer::Background);
    SIF_CHECK(rnd::layer_from_string("Content") == rnd::layer::Content);
    SIF_CHECK(rnd::layer_from_string("Popup") == rnd::layer::Popup);
    SIF_CHECK(rnd::layer_from_string("Modal") == rnd::layer::Modal);
    SIF_CHECK(rnd::layer_from_string("Tooltip") == rnd::layer::Tooltip);
    SIF_CHECK(rnd::layer_from_string("150") == 150);
    SIF_CHECK(rnd::layer_from_string("-20") == -20);

    for (const char* bad : {"modal", "10px", "", "Modal2", "99999999999999999999"}) {
        bool threw = false;
        try {
            (void)rnd::layer_from_string(bad);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        SIF_CHECK(threw);
    }
}

// =================================================================
// ui::Overlay layout
// =================================================================

SIF_TEST(overlay_places_every_child_in_the_same_box) {
    ui::Overlay root;
    root.padding = {10.f, 10.f};

    ui::Rectangle* start = adopt(root, tagged_rect(1));
    ui::Rectangle* end = adopt(root, tagged_rect(2));
    end->h_align = intrnl::Align::End;
    end->v_align = intrnl::Align::End;
    ui::Rectangle* center = adopt(root, tagged_rect(3));
    center->h_align = intrnl::Align::Center;
    center->v_align = intrnl::Align::Center;
    ui::Rectangle* stretch = adopt(root, tagged_rect(4));
    stretch->h_align = intrnl::Align::Stretch;
    stretch->v_align = intrnl::Align::Stretch;
    ui::Rectangle* shifted = adopt(root, tagged_rect(5));
    shifted->h_align = intrnl::Align::End;
    shifted->margin = {5.f, 5.f};
    shifted->offset = {3.f, -4.f};

    lay_out(root, {0.f, 0.f, 200.f, 100.f});

    // Content box is (10, 10, 180, 80): every child is placed inside
    // it, so they overlap rather than being lined up.
    SIF_CHECK(start->result.rect == intrnl::Rect(10.f, 10.f, 20.f, 10.f));
    SIF_CHECK(end->result.rect == intrnl::Rect(170.f, 80.f, 20.f, 10.f));
    SIF_CHECK(center->result.rect == intrnl::Rect(90.f, 45.f, 20.f, 10.f));
    SIF_CHECK(stretch->result.rect == intrnl::Rect(10.f, 10.f, 180.f, 80.f));
    // margin insets the box to (15, 15, 170, 70); offset then moves
    // the End-aligned child by (3, -4) - out of its margin, which is
    // allowed: nothing is clamped.
    SIF_CHECK(shifted->result.rect == intrnl::Rect(168.f, 11.f, 20.f, 10.f));
}

SIF_TEST(overlay_measures_to_its_largest_child_and_honours_explicit_size) {
    ui::Overlay root;
    root.padding = {4.f, 4.f};
    adopt(root, tagged_rect(1, 30.f, 10.f));
    ui::Rectangle* tall = adopt(root, tagged_rect(2, 10.f, 40.f));
    tall->margin = {2.f, 2.f};
    tall->offset = {500.f, 500.f}; // moves, never grows the overlay

    const math::Vector2 natural = root.measure({1000.f, 1000.f});
    SIF_CHECK(natural.x == 38.f); // max(30, 10 + 2*2) + 2*4
    SIF_CHECK(natural.y == 52.f); // max(10, 40 + 2*2) + 2*4

    root.width = intrnl::Size::px(100.f);
    const math::Vector2 fixed = root.measure({1000.f, 1000.f});
    SIF_CHECK(fixed.x == 100.f);
    SIF_CHECK(fixed.y == 52.f);
}

SIF_TEST(overlay_ignores_invisible_children_when_measuring) {
    ui::Overlay root;
    adopt(root, tagged_rect(1, 10.f, 10.f));
    ui::Rectangle* hidden = adopt(root, tagged_rect(2, 300.f, 300.f));
    hidden->visible = false;

    const math::Vector2 size = root.measure({1000.f, 1000.f});
    SIF_CHECK(size.x == 10.f);
    SIF_CHECK(size.y == 10.f);
}

// =================================================================
// Paint order: z_index, layer, visibility
// =================================================================

SIF_TEST(z_index_reorders_siblings_and_ties_keep_document_order) {
    ui::Overlay root;
    adopt(root, tagged_rect(1));
    ui::Rectangle* lifted = adopt(root, tagged_rect(2));
    adopt(root, tagged_rect(3));
    ui::Rectangle* sunk = adopt(root, tagged_rect(4));
    lay_out(root, {0.f, 0.f, 100.f, 100.f});

    SIF_CHECK((drawn_tags(render_tree(root)) == std::vector<int>{1, 2, 3, 4}));

    lifted->z_index = 5;
    sunk->z_index = -1;
    SIF_CHECK((drawn_tags(render_tree(root)) == std::vector<int>{4, 1, 3, 2}));
    SIF_CHECK((root.paint_order() == std::vector<std::size_t>{3, 0, 2, 1}));
}

SIF_TEST(layer_lifts_a_whole_subtree_and_is_inherited_until_overridden) {
    ui::Overlay root;

    // Declared first, so by document order it would be painted
    // first (underneath); its layer puts it - and its children - on top.
    auto dialog_ptr = std::make_unique<ui::Overlay>();
    ui::Overlay* dialog = dialog_ptr.get();
    dialog->layer = rnd::layer::Modal;
    root.add(std::move(dialog_ptr));
    adopt(*dialog, tagged_rect(11));                    // inherits Modal
    ui::Rectangle* backdrop = adopt(*dialog, tagged_rect(12));
    backdrop->layer = rnd::layer::Background;           // nested override

    adopt(root, tagged_rect(2)); // plain content, declared later

    lay_out(root, {0.f, 0.f, 100.f, 100.f});
    const rnd::RenderFrame frame = render_tree(root);

    SIF_CHECK((drawn_tags(frame) == std::vector<int>{12, 2, 11}));
    // The scope was restored after the dialog's subtree: the later
    // sibling is back on Content.
    SIF_CHECK(frame.current_layer() == rnd::layer::Content);
}

SIF_TEST(an_invisible_container_hides_its_whole_subtree) {
    ui::Overlay root;
    auto panel_ptr = std::make_unique<ui::Overlay>();
    ui::Overlay* panel = panel_ptr.get();
    root.add(std::move(panel_ptr));
    adopt(*panel, tagged_rect(1));
    adopt(root, tagged_rect(2));
    lay_out(root, {0.f, 0.f, 100.f, 100.f});

    panel->visible = false;
    SIF_CHECK((drawn_tags(render_tree(root)) == std::vector<int>{2}));

    root.visible = false;
    SIF_CHECK(render_tree(root).temp_items.empty());
}

// =================================================================
// Hit testing
// =================================================================

SIF_TEST(element_at_returns_what_is_drawn_on_top) {
    ui::Overlay root;
    ui::Rectangle* bottom = adopt(root, tagged_rect(1));
    bottom->h_align = intrnl::Align::Stretch;
    bottom->v_align = intrnl::Align::Stretch;
    ui::Rectangle* top = adopt(root, tagged_rect(2, 50.f, 50.f));
    lay_out(root, {0.f, 0.f, 100.f, 100.f});

    const math::Vector2 overlap{10.f, 10.f};
    const math::Vector2 bottom_only{80.f, 80.f};

    SIF_CHECK(root.element_at(overlap) == top);           // later sibling is on top
    SIF_CHECK(root.element_at(bottom_only) == bottom);
    SIF_CHECK(root.element_at({500.f, 500.f}) == nullptr);
    SIF_CHECK((root.hit_path(overlap) == std::vector<ui::UIElement*>{&root, top}));

    bottom->z_index = 1;
    SIF_CHECK(root.element_at(overlap) == bottom);        // z_index wins over document order

    bottom->z_index = 0;
    top->z_index = 10;
    bottom->layer = rnd::layer::Popup;
    SIF_CHECK(root.element_at(overlap) == bottom);        // layer wins over z_index

    bottom->layer.reset();
    top->z_index = 0;
    top->visible = false;
    SIF_CHECK(root.element_at(overlap) == bottom);        // hidden elements are not hit
}

SIF_TEST(hit_testable_false_lets_clicks_through_but_keeps_children_clickable) {
    ui::Overlay root;
    ui::Rectangle* below = adopt(root, tagged_rect(1, 100.f, 100.f));

    auto hud_ptr = std::make_unique<ui::Overlay>();
    ui::Overlay* hud = hud_ptr.get();
    hud->h_align = intrnl::Align::Stretch;
    hud->v_align = intrnl::Align::Stretch;
    hud->hit_testable = false; // full-screen, but must not swallow clicks
    root.add(std::move(hud_ptr));
    ui::Rectangle* hud_button = adopt(*hud, tagged_rect(2, 10.f, 10.f));

    lay_out(root, {0.f, 0.f, 100.f, 100.f});

    SIF_CHECK(root.element_at({50.f, 50.f}) == below);
    SIF_CHECK(root.element_at({5.f, 5.f}) == hud_button);
}

SIF_TEST(element_at_typed_finds_the_clicked_button_but_not_through_a_modal) {
    ui::Overlay root;

    // Declared first and still on top, purely through its layer.
    ui::Rectangle* modal = adopt(root, tagged_rect(9));
    modal->h_align = intrnl::Align::Stretch;
    modal->v_align = intrnl::Align::Stretch;
    modal->layer = rnd::layer::Modal;
    modal->visible = false;

    auto button_ptr = std::make_unique<ui::Button>();
    ui::Button* button = button_ptr.get();
    button->width = intrnl::Size::px(60.f);
    button->height = intrnl::Size::px(30.f);
    root.add(std::move(button_ptr));
    ui::Rectangle* label = adopt(*button, tagged_rect(1, 10.f, 10.f));

    lay_out(root, {0.f, 0.f, 100.f, 100.f});

    const math::Vector2 on_label{2.f, 2.f};
    SIF_CHECK(root.element_at(on_label) == label);
    SIF_CHECK(root.element_at<ui::Button>(on_label) == button);

    // Hidden children are not laid out, so showing one needs a layout
    // pass - which App runs every frame anyway.
    modal->visible = true;
    lay_out(root, {0.f, 0.f, 100.f, 100.f});
    SIF_CHECK(root.element_at(on_label) == modal);
    SIF_CHECK(root.element_at<ui::Button>(on_label) == nullptr); // blocked, not passed through
}

SIF_TEST(menu_handle_click_activates_the_button_drawn_on_top) {
    ui::Menu menu;
    menu.spacing = -20.f; // buttons overlap by 20px

    for (std::uint32_t id : {1u, 2u}) {
        auto b = std::make_unique<ui::Button>();
        b->id = intrnl::RecordID(id);
        b->width = intrnl::Size::px(100.f);
        b->height = intrnl::Size::px(30.f);
        menu.add_button(std::move(b));
    }
    ui::Button* first = menu.find_by_id<ui::Button>(intrnl::RecordID(1));
    ui::Button* second = menu.find_by_id<ui::Button>(intrnl::RecordID(2));
    if (!first || !second) { SIF_CHECK(false); return; }

    ui::UIElement& as_element = menu; // LinearLayout's layout API is protected
    lay_out(as_element, {0.f, 0.f, 200.f, 200.f});

    const math::Vector2 overlap{5.f, 20.f}; // first: y 0..30, second: y 10..40

    SIF_CHECK(menu.handle_click(overlap));
    SIF_CHECK(*menu.active_button_id() == intrnl::RecordID(2)); // later button is on top

    first->z_index = 1;
    SIF_CHECK(menu.handle_click(overlap));
    SIF_CHECK(*menu.active_button_id() == intrnl::RecordID(1));

    second->visible = false;
    SIF_CHECK(!menu.handle_click({5.f, 35.f})); // only the hidden button was there
    SIF_CHECK(*menu.active_button_id() == intrnl::RecordID(1));
}

// =================================================================
// UI XML
// =================================================================

SIF_TEST(xml_overlay_and_layering_attributes_are_parsed) {
    const auto root = build_xml(k_layers_ui_xml);

    SIF_CHECK(dynamic_cast<ui::Overlay*>(root.get()) != nullptr);

    ui::Rectangle* bg = root->find_by_name<ui::Rectangle>("bg");
    ui::Rectangle* badge = root->find_by_name<ui::Rectangle>("badge");
    ui::Rectangle* dialog = root->find_by_name<ui::Rectangle>("dialog");
    ui::Rectangle* tip = root->find_by_name<ui::Rectangle>("tip");
    if (!bg || !badge || !dialog || !tip) { SIF_CHECK(false); return; }

    SIF_CHECK(bg->h_align == intrnl::Align::Stretch);
    SIF_CHECK(bg->v_align == intrnl::Align::Stretch);
    SIF_CHECK(!bg->layer.has_value());

    SIF_CHECK(badge->h_align == intrnl::Align::End);
    SIF_CHECK(badge->margin.x == 5.f && badge->margin.y == 5.f);
    SIF_CHECK(badge->offset.x == 3.f && badge->offset.y == -4.f);
    SIF_CHECK(badge->z_index == 2);

    SIF_CHECK(dialog->layer == rnd::layer::Modal);
    SIF_CHECK(!dialog->visible);
    SIF_CHECK(!dialog->hit_testable);

    SIF_CHECK(tip->layer == 250);
    SIF_CHECK(tip->margin.x == 1.f && tip->margin.y == 2.f);

    lay_out(*root, {0.f, 0.f, 200.f, 100.f});
    SIF_CHECK(badge->result.rect == intrnl::Rect(168.f, 11.f, 20.f, 10.f));

    // bg, then badge (z_index 2) above it; dialog hidden; tip on its
    // own layer 250, above everything else.
    SIF_CHECK((drawn_tags(render_tree(*root)) == std::vector<int>{1, 2, 4}));
}

SIF_TEST(xml_rejects_invalid_layering_attribute_values) {
    SIF_CHECK(rejects(R"(<Overlay><Rectangle layer="Top"/></Overlay>)"));
    SIF_CHECK(rejects(R"(<Overlay><Rectangle visible="yes"/></Overlay>)"));
    SIF_CHECK(rejects(R"(<Overlay><Rectangle hit_testable="0"/></Overlay>)"));
    SIF_CHECK(rejects(R"(<Overlay><Rectangle offset="1|x"/></Overlay>)"));
    SIF_CHECK(rejects(R"(<Overlay><Rectangle margin=""/></Overlay>)"));
    SIF_CHECK(rejects(R"(<Overlay><Rectangle h_align="Left"/></Overlay>)"));
    SIF_CHECK(rejects(R"(<Overlay><Rectangle z_index="top"/></Overlay>)"));

    SIF_CHECK(!rejects(R"(<Overlay><Rectangle layer="-5" visible="true" offset="2" z_index="-3"/></Overlay>)"));
}

SIF_TEST(xml_justify_is_parsed_and_validated) {
    const auto root = build_xml(R"(<HBox justify="SpaceBetween"/>)");
    const auto* row = dynamic_cast<ui::LinearLayout*>(root.get());
    SIF_CHECK(row != nullptr && row->justify == intrnl::Justify::SpaceBetween);
    SIF_CHECK(rejects(R"(<HBox justify="Spread"/>)"));
}
