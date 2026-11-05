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

#include "sif/layout_engine/Parser.h"
#include "sif/layout_engine/Tokenizer.h"
#include "sif/layout_engine/UIFactory.h"
#include "sif/layout_engine/elements/Overlay.h"
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/layout_engine/elements/ScrollView.h"
#include "sif/layout_engine/elements/V_HBox.h"
#include "sif/render/FrameContext.h"
#include "sif/render/RecordingRenderer.h"
#include "sif/render/elements/Rectangle.h"

using namespace sif;

namespace {
    std::unique_ptr<ui::Rectangle> px_rect(const float w, const float h, const std::uint8_t tag = 0) {
        auto r = std::make_unique<ui::Rectangle>(intrnl::Color(tag, 0, 0));
        r->width = intrnl::Size::px(w);
        r->height = intrnl::Size::px(h);
        return r;
    }

    template<typename T>
    T* adopt(ui::UIElement& parent, std::unique_ptr<T> child) {
        T* raw = child.get();
        parent.add(std::move(child));
        return raw;
    }

    std::vector<rnd::DrawCall> draw(const ui::UIElement& root) {
        rnd::RenderFrame frame;
        root.render(frame, rnd::FrameContext(true));
        rnd::RecordingRenderer renderer;
        renderer.render(frame);
        return renderer.last_frame();
    }

    /// @brief A ScrollView of 10 rows, 30 px each, in a 100 px tall viewport.
    struct ScrollFixture {
        ui::Overlay root;
        ui::ScrollView* view = nullptr;
        std::vector<ui::Rectangle*> rows;

        ScrollFixture() {
            view = adopt(root, std::make_unique<ui::ScrollView>());
            view->width = intrnl::Size::px(200.f);
            view->height = intrnl::Size::px(100.f);
            auto* list = adopt(*view, std::make_unique<ui::VBox>());
            for (std::uint8_t i = 0; i < 10; ++i) {
                rows.push_back(adopt(*list, px_rect(200.f, 30.f, static_cast<std::uint8_t>(i + 1))));
            }
            relayout();
        }

        void relayout() {
            root.measure({800.f, 600.f});
            root.layout({0.f, 0.f, 800.f, 600.f});
        }
    };
}

SIF_TEST(clip_scopes_nest_by_intersection_and_restore) {
    rnd::RenderFrame frame;
    SIF_CHECK(!frame.current_clip().has_value());
    {
        const rnd::ClipScope outer(frame, {0.f, 0.f, 100.f, 100.f});
        {
            const rnd::ClipScope inner(frame, {50.f, 50.f, 100.f, 100.f});
            frame.submit(std::make_unique<rnd::Rectangle>());
        }
        SIF_CHECK(frame.current_clip() == intrnl::Rect(0.f, 0.f, 100.f, 100.f));
    }
    SIF_CHECK(!frame.current_clip().has_value());
    SIF_CHECK(frame.temp_items[0]->clip == intrnl::Rect(50.f, 50.f, 50.f, 50.f));

    SIF_CHECK(intrnl::Rect(0, 0, 10, 10).intersection({20, 20, 5, 5}) == intrnl::Rect(20, 20, 0, 0)); // empty, never negative
}

SIF_TEST(a_clipping_element_clips_its_subtree_and_its_hit_testing) {
    ui::Overlay root;
    auto* box = adopt(root, std::make_unique<ui::Overlay>());
    box->width = intrnl::Size::px(50.f);
    box->height = intrnl::Size::px(50.f);
    box->clip = true;
    auto* child = adopt(*box, px_rect(200.f, 200.f, 7)); // overflows the box
    root.measure({800.f, 600.f});
    root.layout({0.f, 0.f, 800.f, 600.f});

    const auto calls = draw(root);
    SIF_CHECK(calls.size() == 1 && calls[0].clip == intrnl::Rect(0.f, 0.f, 50.f, 50.f));

    SIF_CHECK(root.element_at({10.f, 10.f}) == child);
    SIF_CHECK(root.element_at({150.f, 150.f}) == &root); // the overflow is not clickable

    box->clip = false;
    SIF_CHECK(root.element_at({150.f, 150.f}) == child);
    SIF_CHECK(!draw(root)[0].clip.has_value());
}

SIF_TEST(scroll_view_lays_content_out_at_its_natural_size) {
    ScrollFixture f;
    SIF_CHECK(f.view->result.rect.height == 100.f);
    SIF_CHECK(f.view->content_size().y == 300.f);
    SIF_CHECK(f.view->max_scroll().y == 200.f);
    SIF_CHECK(f.rows[3]->result.rect.y == 90.f);
}

SIF_TEST(scroll_view_scrolls_clamps_and_moves_hit_testing_with_it) {
    ScrollFixture f;
    f.view->scroll_by({0.f, 50.f});
    SIF_CHECK(f.view->scroll_offset().y == 50.f);
    SIF_CHECK(f.rows[3]->result.rect.y == 40.f); // re-placed immediately
    SIF_CHECK(f.root.element_at({10.f, 5.f}) == f.rows[1]); // row 1 spans 30..60, now -20..10

    f.view->scroll_by({0.f, 1000.f});
    SIF_CHECK(f.view->scroll_offset().y == 200.f); // clamped to the end
    SIF_CHECK(f.root.element_at({10.f, 5.f}) == f.rows[6]);
    SIF_CHECK(f.root.element_at({10.f, 150.f}) == &f.root); // row 9 continues below, but is clipped

    f.view->scroll_to({0.f, -10.f});
    SIF_CHECK(f.view->scroll_offset().y == 0.f);

    // Wheel: one notch up at the start does nothing, one notch down scrolls.
    SIF_CHECK(!f.view->handle_wheel(1.f));
    SIF_CHECK(f.view->handle_wheel(-1.f) && f.view->scroll_offset().y == 40.f);

    // A layout pass keeps the offset (and re-clamps it if the content shrank).
    f.relayout();
    SIF_CHECK(f.view->scroll_offset().y == 40.f);

    // Every row is drawn clipped to the viewport.
    for (const rnd::DrawCall& call : draw(f.root)) {
        SIF_CHECK(call.clip == intrnl::Rect(0.f, 0.f, 200.f, 100.f));
    }
}

SIF_TEST(scroll_view_gives_auto_content_the_viewport_length) {
    ui::ScrollView view;
    view.width = intrnl::Size::px(100.f);
    view.height = intrnl::Size::px(80.f);
    auto* filler = adopt(view, std::make_unique<ui::Rectangle>()); // Auto: asks for "everything"
    view.measure({500.f, 500.f});
    view.layout({0.f, 0.f, 100.f, 80.f});
    SIF_CHECK(filler->result.rect.height == 80.f);
    SIF_CHECK(view.max_scroll().y == 0.f);
}

SIF_TEST(xml_scroll_view_and_clip_are_parsed) {
    sif::test::fixtures::write_file("sif_test_scroll.ui.xml",
        R"(<Overlay clip="true"><ScrollView name="s" direction="Horizontal" scroll_step="12" height="PX|50"/></Overlay>)");
    const auto node = ui::Parser::parse(ui::Tokenizer::tokenize("sif_test_scroll.ui.xml"));
    ui::UIFactory::instance().set_strict(true);
    const auto root = ui::UIFactory::instance().build(*node);
    ui::UIFactory::instance().set_strict(false);
    auto* s = root->find_by_name<ui::ScrollView>("s");
    SIF_CHECK(root->clip);
    SIF_CHECK(s != nullptr && s->direction == ui::ScrollDirection::Horizontal && s->scroll_step == 12.f && s->clip);
}
