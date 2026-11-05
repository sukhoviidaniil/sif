/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/WorldView.h"

#include <algorithm>

namespace sif::ui {
WorldView::WorldView() {
    // The world is not allowed to draw over whatever surrounds the panel.
    clip = true;
}

math::Vector2 WorldView::measure(const math::Vector2& available) {
    math::Vector2 size = resolve_size(available);
    size.x = std::max(0.f, size.x);
    size.y = std::max(0.f, size.y);
    return size;
}

void WorldView::layout(const intrnl::Rect r) {
    result.rect = r;
    if (r.width > 0.f && r.height > 0.f) {
        camera.set_viewport(r);
    }

    for (auto& c : children) {
        if (!c->visible)
            continue;
        const intrnl::Rect screen = c->world_rect.has_value() ? camera.world_to_screen(*c->world_rect) : r;
        // Measured first, so containers used as world objects can size
        // their own children against the projected rect.
        c->measure({screen.width, screen.height});
        c->layout(screen);
    }
}

bool WorldView::culled(const UIElement& child) const {
    return child.world_rect.has_value() && !camera.is_visible(*child.world_rect);
}

void WorldView::append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const {
    for (const std::size_t i : paint_order()) {
        if (!culled(*children[i])) {
            children[i]->render(frame, ctx);
        }
    }
}

math::Point2 WorldView::to_world(const math::Vector2 screen_point) const {
    return camera.screen_to_world({screen_point.x, screen_point.y});
}
} // namespace sif::ui
