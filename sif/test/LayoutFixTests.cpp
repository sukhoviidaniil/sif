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

#include <cmath>
#include <memory>
#include <vector>

#include "sif/internal/Size.h"
#include "sif/layout_engine/elements/Button.h"
#include "sif/layout_engine/elements/Menu.h"
#include "sif/layout_engine/elements/Overlay.h"
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/layout_engine/elements/V_HBox.h"

using namespace sif;

namespace {
    /// @brief A fixed-size Rectangle (sizes in pixels).
    std::unique_ptr<ui::Rectangle> px_rect(const float w, const float h) {
        auto r = std::make_unique<ui::Rectangle>();
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

    /**
     * @brief Measures and lays out through the UIElement interface.
     *
     * LinearLayout declares measure/layout protected, so tests reach
     * them the same way a parent container (or App) does.
     */
    math::Vector2 measure(ui::UIElement& e, const math::Vector2 available) { return e.measure(available); }

    void lay_out(ui::UIElement& e, const intrnl::Rect rect) {
        e.measure({rect.width, rect.height});
        e.layout(rect);
    }
}

// =================================================================
// PERCENT sizes (audit 1.1)
// =================================================================

SIF_TEST(size_resolve_treats_percent_as_0_to_100) {
    SIF_CHECK(intrnl::Size("PERCENT|50").resolve(800.f) == 400.f);
    SIF_CHECK(intrnl::Size("PERCENT|100").resolve(800.f) == 800.f);
    SIF_CHECK(intrnl::Size::percent(25.f).resolve(200.f) == 50.f);
    SIF_CHECK(intrnl::Size("PX|30").resolve(800.f) == 30.f);
    SIF_CHECK(intrnl::Size::auto_().resolve(800.f) == 800.f);
}

SIF_TEST(every_element_resolves_percent_the_same_way) {
    // UIElement::resolve_size (via the base vertical stacking layout)
    ui::UIElement root;
    ui::Rectangle* rect = adopt(root, std::make_unique<ui::Rectangle>());
    rect->width = intrnl::Size("PERCENT|50");
    rect->height = intrnl::Size("PX|10");
    lay_out(root, {0.f, 0.f, 800.f, 600.f});
    SIF_CHECK(rect->result.rect.width == 400.f); // was 40000

    // Rectangle::measure
    SIF_CHECK(rect->measure({800.f, 600.f}).x == 400.f);

    // Button::measure
    ui::Button button;
    button.width = intrnl::Size("PERCENT|25");
    button.height = intrnl::Size("PERCENT|10");
    const math::Vector2 b = button.measure({800.f, 600.f});
    SIF_CHECK(b.x == 200.f && b.y == 60.f);

    // Overlay / LinearLayout own size
    ui::Overlay overlay;
    overlay.width = intrnl::Size("PERCENT|50");
    SIF_CHECK(overlay.measure({800.f, 600.f}).x == 400.f);

    ui::VBox column;
    column.height = intrnl::Size("PERCENT|50");
    SIF_CHECK(measure(column, {800.f, 600.f}).y == 300.f);
}

// =================================================================
// LinearLayout padding / margin / own size (audit 1.5)
// =================================================================

SIF_TEST(linear_layout_without_padding_or_margin_is_unchanged) {
    // Hand-computed with the pre-fix algorithm: the fix must not move
    // anything in a scene that sets neither padding nor margin.
    ui::HBox row;
    row.spacing = 4.f;
    row.align = intrnl::Align::Center;
    ui::Rectangle* a = adopt(row, px_rect(20.f, 10.f));
    ui::Rectangle* b = adopt(row, px_rect(30.f, 40.f));
    ui::Rectangle* grow = adopt(row, px_rect(5.f, 5.f));
    grow->flex = 1.f;

    lay_out(row, {10.f, 20.f, 200.f, 50.f});

    SIF_CHECK(a->result.rect == intrnl::Rect(10.f, 40.f, 20.f, 10.f));
    SIF_CHECK(b->result.rect == intrnl::Rect(34.f, 25.f, 30.f, 40.f));
    SIF_CHECK(grow->result.rect == intrnl::Rect(68.f, 42.5f, 142.f, 5.f));
}

SIF_TEST(linear_layout_applies_padding) {
    ui::VBox column;
    column.padding = {10.f, 10.f};
    column.spacing = 5.f;
    ui::Rectangle* first = adopt(column, px_rect(50.f, 20.f));
    ui::Rectangle* second = adopt(column, px_rect(30.f, 10.f));

    const math::Vector2 size = measure(column, {1000.f, 1000.f});
    SIF_CHECK(size.x == 70.f); // max(50, 30) + 2 * 10
    SIF_CHECK(size.y == 55.f); // 20 + 5 + 10 + 2 * 10

    lay_out(column, {0.f, 0.f, 200.f, 100.f});
    SIF_CHECK(first->result.rect == intrnl::Rect(10.f, 10.f, 50.f, 20.f));
    SIF_CHECK(second->result.rect == intrnl::Rect(10.f, 35.f, 30.f, 10.f));
}

SIF_TEST(linear_layout_applies_child_margins) {
    ui::HBox row;
    ui::Rectangle* a = adopt(row, px_rect(20.f, 10.f));
    a->margin = {3.f, 4.f};
    ui::Rectangle* b = adopt(row, px_rect(10.f, 10.f));

    const math::Vector2 size = measure(row, {1000.f, 1000.f});
    SIF_CHECK(size.x == 36.f); // 20 + 2*3 + 10
    SIF_CHECK(size.y == 18.f); // max(10 + 2*4, 10)

    lay_out(row, {0.f, 0.f, 100.f, 50.f});
    SIF_CHECK(a->result.rect == intrnl::Rect(3.f, 4.f, 20.f, 10.f));
    SIF_CHECK(b->result.rect == intrnl::Rect(26.f, 0.f, 10.f, 10.f));
}

SIF_TEST(linear_layout_flex_shares_what_is_left_after_margins) {
    ui::HBox row;
    ui::Rectangle* grow = adopt(row, px_rect(1.f, 10.f));
    grow->flex = 1.f;
    grow->margin = {5.f, 0.f};
    ui::Rectangle* fixed = adopt(row, px_rect(20.f, 10.f));

    lay_out(row, {0.f, 0.f, 100.f, 10.f});
    SIF_CHECK(grow->result.rect == intrnl::Rect(5.f, 0.f, 70.f, 10.f)); // 100 - 2*5 - 20
    SIF_CHECK(fixed->result.rect == intrnl::Rect(80.f, 0.f, 20.f, 10.f));
}

SIF_TEST(linear_layout_stretch_and_center_respect_cross_margins_and_padding) {
    ui::VBox column;
    column.align = intrnl::Align::Stretch;
    column.padding = {6.f, 6.f};
    ui::Rectangle* wide = adopt(column, px_rect(10.f, 10.f));
    wide->margin = {4.f, 0.f};

    lay_out(column, {0.f, 0.f, 112.f, 50.f});
    SIF_CHECK(wide->result.rect == intrnl::Rect(10.f, 6.f, 92.f, 10.f)); // 112 - 2*6 - 2*4

    column.align = intrnl::Align::Center;
    lay_out(column, {0.f, 0.f, 112.f, 50.f});
    SIF_CHECK(wide->result.rect == intrnl::Rect(51.f, 6.f, 10.f, 10.f)); // 6 + 4 + (92 - 10) / 2
}

SIF_TEST(linear_layout_honours_its_own_explicit_size) {
    ui::VBox column;
    adopt(column, px_rect(10.f, 10.f));

    column.height = intrnl::Size::px(300.f);
    column.width = intrnl::Size("PERCENT|50");
    const math::Vector2 size = measure(column, {400.f, 1000.f});
    SIF_CHECK(size.x == 200.f);
    SIF_CHECK(size.y == 300.f);

    // ...so a parent now gives it the space it asked for.
    ui::VBox parent;
    ui::VBox* child = adopt(parent, std::make_unique<ui::VBox>());
    child->height = intrnl::Size::px(120.f);
    ui::Rectangle* after = adopt(parent, px_rect(10.f, 10.f));
    lay_out(parent, {0.f, 0.f, 400.f, 1000.f});
    SIF_CHECK(child->result.rect.height == 120.f);
    SIF_CHECK(after->result.rect.y == 120.f);
}

SIF_TEST(menu_inherits_the_padding_fix) {
    ui::Menu menu;
    menu.padding = {8.f, 8.f};
    auto b = std::make_unique<ui::Button>();
    b->id = intrnl::RecordID(1);
    b->width = intrnl::Size::px(100.f);
    b->height = intrnl::Size::px(30.f);
    ui::Button& button = menu.add_button(std::move(b));

    lay_out(menu, {0.f, 0.f, 300.f, 300.f});
    SIF_CHECK(button.result.rect == intrnl::Rect(8.f, 8.f, 100.f, 30.f));
}

SIF_TEST(linear_layout_never_reports_a_negative_size) {
    // Negative spacing larger than the content used to make the row
    // report a negative width, which pulled later siblings over it.
    ui::HBox row;
    row.spacing = -50.f;
    adopt(row, px_rect(10.f, 10.f));
    adopt(row, px_rect(10.f, 10.f));

    const math::Vector2 size = measure(row, {1000.f, 1000.f});
    SIF_CHECK(size.x == 0.f);
    SIF_CHECK(size.y == 10.f);
}

// =================================================================
// Row/column measurement never exceeds what it was offered (audit 2.12)
// =================================================================

SIF_TEST(a_row_of_auto_children_measures_to_the_available_width_not_beyond) {
    ui::HBox strip;
    strip.spacing = 8.f;
    for (int i = 0; i < 8; ++i) {
        adopt(strip, std::make_unique<ui::Rectangle>())->flex = 1.f; // Auto + flex, like the demo's tile strip
    }

    // Used to be 968 + 7 * 8 = 1024: wider than the parent offered.
    SIF_CHECK(measure(strip, {968.f, 100.f}).x == 968.f);

    // In a column, the strip now gets exactly the column's width and
    // its last tile ends at the column's edge.
    ui::VBox column;
    ui::HBox* row = adopt(column, std::make_unique<ui::HBox>());
    row->spacing = 8.f;
    ui::Rectangle* last = nullptr;
    for (int i = 0; i < 8; ++i) {
        last = adopt(*row, std::make_unique<ui::Rectangle>());
        last->flex = 1.f;
    }
    lay_out(column, {0.f, 0.f, 968.f, 100.f});
    SIF_CHECK(row->result.rect.width == 968.f);
    SIF_CHECK(last->result.rect.x + last->result.rect.width == 968.f);
}

SIF_TEST(children_that_do_not_fit_still_overflow_but_the_row_does_not_grow) {
    ui::HBox row;
    ui::Rectangle* a = adopt(row, px_rect(80.f, 10.f));
    ui::Rectangle* b = adopt(row, px_rect(80.f, 10.f));

    SIF_CHECK(measure(row, {100.f, 50.f}).x == 100.f);

    lay_out(row, {0.f, 0.f, 100.f, 50.f});
    SIF_CHECK(a->result.rect == intrnl::Rect(0.f, 0.f, 80.f, 10.f));
    SIF_CHECK(b->result.rect == intrnl::Rect(80.f, 0.f, 80.f, 10.f)); // placed as before, past the edge
}

// =================================================================
// justify (audit 2.3)
// =================================================================

namespace {
    /// @brief x of each of three 20px children in a 200px row, for one justify mode.
    std::vector<float> xs_for(const intrnl::Justify justify, const bool with_flex = false) {
        ui::HBox row;
        row.justify = justify;
        std::vector<ui::Rectangle*> kids;
        for (int i = 0; i < 3; ++i) {
            kids.push_back(adopt(row, px_rect(20.f, 10.f)));
        }
        if (with_flex) {
            kids[1]->flex = 1.f;
        }
        lay_out(row, {0.f, 0.f, 200.f, 10.f});
        std::vector<float> xs;
        for (const ui::Rectangle* k : kids) {
            xs.push_back(k->result.rect.x);
        }
        return xs;
    }
}

SIF_TEST(justify_distributes_leftover_main_axis_space) {
    // 200 - 3 * 20 = 140 px left over.
    SIF_CHECK((xs_for(intrnl::Justify::Start) == std::vector<float>{0.f, 20.f, 40.f}));
    SIF_CHECK((xs_for(intrnl::Justify::Center) == std::vector<float>{70.f, 90.f, 110.f}));
    SIF_CHECK((xs_for(intrnl::Justify::End) == std::vector<float>{140.f, 160.f, 180.f}));
    SIF_CHECK((xs_for(intrnl::Justify::SpaceBetween) == std::vector<float>{0.f, 90.f, 180.f}));
    SIF_CHECK((xs_for(intrnl::Justify::SpaceEvenly) == std::vector<float>{35.f, 90.f, 145.f}));
    const std::vector<float> around = xs_for(intrnl::Justify::SpaceAround); // gap 140/3, half at the edges
    SIF_CHECK(std::abs(around[0] - 70.f / 3.f) < 1e-3f && std::abs(around[1] - 90.f) < 1e-3f);
}

SIF_TEST(justify_has_no_effect_when_a_flex_child_takes_the_space) {
    SIF_CHECK(xs_for(intrnl::Justify::Center, true) == xs_for(intrnl::Justify::Start, true));
}
