/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/Overlay.h"

#include <algorithm>

namespace sif::ui {
math::Vector2 Overlay::measure(const math::Vector2& available) {
    const math::Vector2 inner{std::max(0.f, available.x - padding.x * 2), std::max(0.f, available.y - padding.y * 2)};

    math::Vector2 content{0.f, 0.f};

    for (auto& c : children) {
        if (!c->visible)
            continue;

        // Every child is offered the same box (minus its own margin):
        // they share the space instead of splitting it.
        const math::Vector2 child_available{std::max(0.f, inner.x - c->margin.x * 2),
                                            std::max(0.f, inner.y - c->margin.y * 2)};
        const math::Vector2 cs = c->measure(child_available);

        content.x = std::max(content.x, cs.x + c->margin.x * 2);
        content.y = std::max(content.y, cs.y + c->margin.y * 2);
    }

    math::Vector2 size{content.x + padding.x * 2, content.y + padding.y * 2};

    if (width.type != intrnl::Size::Type::Auto || height.type != intrnl::Size::Type::Auto) {
        const math::Vector2 fixed = resolve_size(available);
        if (width.type != intrnl::Size::Type::Auto)
            size.x = fixed.x;
        if (height.type != intrnl::Size::Type::Auto)
            size.y = fixed.y;
    }

    size.x = std::clamp(size.x, min_size.x, max_size.x);
    size.y = std::clamp(size.y, min_size.y, max_size.y);

    return size;
}

void Overlay::layout(const intrnl::Rect r) {
    result.rect = r;

    const intrnl::Rect content{r.x + padding.x, r.y + padding.y, std::max(0.f, r.width - padding.x * 2),
                               std::max(0.f, r.height - padding.y * 2)};

    for (auto& c : children) {
        if (!c->visible)
            continue;

        const intrnl::Rect box{content.x + c->margin.x, content.y + c->margin.y,
                               std::max(0.f, content.width - c->margin.x * 2),
                               std::max(0.f, content.height - c->margin.y * 2)};

        const math::Vector2 desired = c->measure({box.width, box.height});

        const math::Vector2 h = place(c->h_align, box.x, box.width, desired.x);
        const math::Vector2 v = place(c->v_align, box.y, box.height, desired.y);

        c->layout({h.x + c->offset.x, v.x + c->offset.y, h.y, v.y});
    }
}

math::Vector2 Overlay::place(const intrnl::Align align, const float start, const float extent, const float desired) {
    const float length = std::max(0.f, desired);

    switch (align) {
    case intrnl::Align::Stretch:
        return {start, extent};
    case intrnl::Align::Center:
        return {start + (extent - length) * 0.5f, length};
    case intrnl::Align::End:
        return {start + extent - length, length};
    case intrnl::Align::Start:
    default:
        return {start, length};
    }
}
} // namespace sif::ui
