/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/ScrollView.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace sif::ui {
namespace {
/**
 * @brief A measured length, or the viewport's when the content asked
 * for (practically) all of the unlimited space it was offered.
 *
 * Auto leaves measure as "everything available", which along a scroll
 * axis is infinity - clamped to the default max_size of 1e9 on the way
 * out. Either way it means "no natural length", not "a billion pixels".
 */
float bounded(const float measured, const float viewport) {
    if (!std::isfinite(measured) || measured >= 0.5e9f) {
        return viewport;
    }
    return measured;
}
} // namespace

ScrollView::ScrollView() { clip = true; }

bool ScrollView::scrolls_x() const { return direction != ScrollDirection::Vertical; }

bool ScrollView::scrolls_y() const { return direction != ScrollDirection::Horizontal; }

math::Vector2 ScrollView::measure(const math::Vector2& available) {
    math::Vector2 size = resolve_size(available);
    size.x = std::max(0.f, size.x);
    size.y = std::max(0.f, size.y);
    return size;
}

intrnl::Rect ScrollView::viewport() const {
    const intrnl::Rect& r = result.rect;
    return {r.x + padding.x, r.y + padding.y, std::max(0.f, r.width - padding.x * 2),
            std::max(0.f, r.height - padding.y * 2)};
}

void ScrollView::layout(const intrnl::Rect r) {
    // Measured against the *previous* content size, before it changes.
    const math::Vector2 old_max = max_scroll();
    const bool was_at_end_x = offset_.x >= old_max.x - 0.5f;
    const bool was_at_end_y = offset_.y >= old_max.y - 0.5f;

    result.rect = r;
    const intrnl::Rect view = viewport();

    constexpr float unlimited = std::numeric_limits<float>::infinity();
    const math::Vector2 offered{scrolls_x() ? unlimited : view.width, scrolls_y() ? unlimited : view.height};

    // Measure first: the content's size decides how far the view may
    // scroll, and the offset must be clamped before anything is placed.
    std::vector<math::Vector2> sizes;
    sizes.reserve(children.size());
    content_size_ = {view.width, view.height};
    for (auto& c : children) {
        math::Vector2 s{0.f, 0.f};
        if (c->visible) {
            const math::Vector2 m = c->measure(offered);
            s = {scrolls_x() ? bounded(m.x, view.width) : view.width,
                 scrolls_y() ? bounded(m.y, view.height) : view.height};
            content_size_.x = std::max(content_size_.x, s.x);
            content_size_.y = std::max(content_size_.y, s.y);
        }
        sizes.push_back(s);
    }

    if (follow_end) {
        const math::Vector2 new_max = max_scroll();
        if (was_at_end_x)
            offset_.x = new_max.x;
        if (was_at_end_y)
            offset_.y = new_max.y;
    }
    clamp_offset();

    for (std::size_t i = 0; i < children.size(); ++i) {
        if (!children[i]->visible)
            continue;
        children[i]->layout({view.x - offset_.x, view.y - offset_.y, sizes[i].x, sizes[i].y});
    }
}

math::Vector2 ScrollView::max_scroll() const {
    const intrnl::Rect view = viewport();
    return {std::max(0.f, content_size_.x - view.width), std::max(0.f, content_size_.y - view.height)};
}

void ScrollView::clamp_offset() {
    const math::Vector2 limit = max_scroll();
    offset_.x = scrolls_x() ? std::clamp(offset_.x, 0.f, limit.x) : 0.f;
    offset_.y = scrolls_y() ? std::clamp(offset_.y, 0.f, limit.y) : 0.f;
}

void ScrollView::scroll_by(const math::Vector2 delta) { scroll_to({offset_.x + delta.x, offset_.y + delta.y}); }

void ScrollView::scroll_to(const math::Vector2 target) {
    offset_ = target;
    clamp_offset();
    // Re-place the content right away, so a scroll made between two
    // frames (e.g. by an input handler) is already reflected in rects
    // and hit-testing, not only after the next layout pass.
    if (result.rect.width > 0.f || result.rect.height > 0.f) {
        layout(result.rect);
    }
}

bool ScrollView::handle_wheel(const float wheel_delta) {
    const math::Vector2 before = offset_;
    const float distance = -wheel_delta * scroll_step;
    if (scrolls_y()) {
        scroll_by({0.f, distance});
    } else {
        scroll_by({distance, 0.f});
    }
    return offset_.x != before.x || offset_.y != before.y;
}

bool ScrollView::on_ui_event(UIEvent& e) {
    if (e.type == UIEventType::Wheel && handle_wheel(e.wheel_delta)) {
        e.handled = true;
        return true;
    }
    return UIElement::on_ui_event(e);
}

math::Vector2 ScrollView::scroll_offset() const { return offset_; }

math::Vector2 ScrollView::content_size() const { return content_size_; }
} // namespace sif::ui
