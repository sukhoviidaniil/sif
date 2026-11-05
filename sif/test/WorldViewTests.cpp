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
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/layout_engine/elements/WorldView.h"
#include "sif/layout_engine/internal/add_attribute.h"
#include "sif/render/FrameContext.h"
#include "sif/render/RecordingRenderer.h"

using namespace sif;

namespace {
    ui::Rectangle* world_rect(ui::WorldView& view, const intrnl::Rect rect, const std::uint8_t tag) {
        auto r = std::make_unique<ui::Rectangle>(intrnl::Color(tag, 0, 0));
        r->world_rect = rect;
        ui::Rectangle* raw = r.get();
        view.add(std::move(r));
        return raw;
    }

    void lay_out(ui::WorldView& view, const intrnl::Rect rect) {
        view.measure({rect.width, rect.height});
        view.layout(rect);
    }

    std::vector<int> drawn_tags(const ui::WorldView& view) {
        rnd::RenderFrame frame;
        view.render(frame, rnd::FrameContext(true));
        rnd::RecordingRenderer recorder;
        recorder.render(frame);
        std::vector<int> tags;
        for (const rnd::DrawCall& call : recorder.last_frame()) {
            tags.push_back(call.color.r);
        }
        return tags;
    }
}

SIF_TEST(world_view_projects_children_through_its_camera) {
    ui::WorldView view;
    ui::Rectangle* corner = world_rect(view, {-1.f, -1.f, 1.f, 1.f}, 1);
    ui::Rectangle* middle = world_rect(view, {0.f, 0.f, 0.5f, 0.5f}, 2);
    auto background = std::make_unique<ui::Rectangle>();
    ui::Rectangle* bg = background.get();
    view.add(std::move(background)); // no world_rect: fills the view

    lay_out(view, {10.f, 20.f, 200.f, 200.f}); // world [-1, 1] -> [10, 210] x [20, 220]
    SIF_CHECK(corner->result.rect == intrnl::Rect(10.f, 20.f, 100.f, 100.f));
    SIF_CHECK(middle->result.rect == intrnl::Rect(110.f, 120.f, 50.f, 50.f));
    SIF_CHECK(bg->result.rect == intrnl::Rect(10.f, 20.f, 200.f, 200.f));

    // Moving and zooming the camera moves the world, not the panel.
    view.camera.set_center({0.5f, 0.5f});
    view.camera.set_zoom(2.f);
    lay_out(view, {10.f, 20.f, 200.f, 200.f});
    SIF_CHECK(middle->result.rect == intrnl::Rect(10.f, 20.f, 100.f, 100.f));
    SIF_CHECK(view.result.rect == intrnl::Rect(10.f, 20.f, 200.f, 200.f));

    const math::Point2 world = view.to_world({110.f, 120.f}); // the panel's centre
    SIF_CHECK(world.x == 0.5f && world.y == 0.5f);
}

SIF_TEST(world_view_culls_what_the_camera_cannot_see_and_clips_the_rest) {
    ui::WorldView view;
    world_rect(view, {-0.1f, -0.1f, 0.2f, 0.2f}, 1);
    ui::Rectangle* far_away = world_rect(view, {5.f, 5.f, 0.2f, 0.2f}, 2);
    world_rect(view, {0.9f, 0.f, 0.5f, 0.1f}, 3); // half outside: drawn, but clipped
    lay_out(view, {0.f, 0.f, 100.f, 100.f});

    SIF_CHECK(view.culled(*far_away));
    SIF_CHECK((drawn_tags(view) == std::vector<int>{1, 3}));

    rnd::RenderFrame frame;
    view.render(frame, rnd::FrameContext(true));
    SIF_CHECK(frame.temp_items.size() == 2 && frame.temp_items[1]->clip == intrnl::Rect(0.f, 0.f, 100.f, 100.f));

    view.camera.set_center({5.1f, 5.1f});
    lay_out(view, {0.f, 0.f, 100.f, 100.f});
    SIF_CHECK((drawn_tags(view) == std::vector<int>{2}));
}

SIF_TEST(xml_world_view_and_world_rect_are_parsed) {
    sif::test::fixtures::write_file("sif_test_world.ui.xml",
        R"(<WorldView aspect="Fill" zoom="2" center="0.5|-0.25" y_axis_up="true">
    <Rectangle name="r" world_rect="0|0|0.1|0.2"/>
</WorldView>)");
    const auto node = ui::Parser::parse(ui::Tokenizer::tokenize("sif_test_world.ui.xml"));
    ui::UIFactory::instance().set_strict(true);
    const auto root = ui::UIFactory::instance().build(*node);
    ui::UIFactory::instance().set_strict(false);

    auto* view = dynamic_cast<ui::WorldView*>(root.get());
    auto* r = root->find_by_name<ui::Rectangle>("r");
    if (!view || !r) { SIF_CHECK(false); return; }
    SIF_CHECK(view->camera.aspect_policy() == rnd::AspectPolicy::Fill);
    SIF_CHECK(view->camera.zoom() == 2.f);
    SIF_CHECK(view->camera.center().x == 0.5f && view->camera.center().y == -0.25f);
    SIF_CHECK(view->camera.y_axis_up());
    SIF_CHECK(r->world_rect == intrnl::Rect(0.f, 0.f, 0.1f, 0.2f));

    SIF_CHECK(ui::parse_rect("1|2|3|4") == intrnl::Rect(1.f, 2.f, 3.f, 4.f));
    for (const char* bad : {"1|2|3", "1|2|3|4|5", "a|2|3|4", ""}) {
        bool threw = false;
        try { (void)ui::parse_rect(bad); } catch (const std::invalid_argument&) { threw = true; }
        SIF_CHECK(threw);
    }
}
