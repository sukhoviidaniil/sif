/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/Rectangle.h"

#include "sif/render/elements/Rectangle.h"

#include <algorithm>

namespace sif::ui {
Rectangle::Rectangle() = default;

Rectangle::Rectangle(const intrnl::Color fill, const intrnl::Color border, const int border_w)
    : fill_color(fill), border_color(border), border_width(border_w) {}

math::Vector2 Rectangle::measure(const math::Vector2& available) {
    math::Vector2 size{std::max(0.f, available.x), std::max(0.f, available.y)};

    if (width.type != intrnl::Size::Type::Auto)
        size.x = width.resolve(available.x);

    if (height.type != intrnl::Size::Type::Auto)
        size.y = height.resolve(available.y);

    const float bw = static_cast<float>(border_width) * 2.f;
    size.x += bw;
    size.y += bw;

    size.x = std::clamp(size.x, min_size.x, max_size.x);
    size.y = std::clamp(size.y, min_size.y, max_size.y);

    return size;
}

void Rectangle::append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& /*ctx*/) const {
    if (!visible)
        return;

    auto item = std::make_unique<rnd::Rectangle>();
    item->rect = result.rect;
    item->color = fill_color;
    item->border_color = border_color;
    item->border_width = border_width;

    // submit(), not constant_items: the two buckets are a *caching*
    // distinction (rebuilt every frame vs. not), not a z-order. Using
    // them as one meant every animated entity was painted on top of
    // every label - a pause overlay ended up underneath the bombs it
    // was supposed to cover. Covering is now expressed explicitly:
    // submit() stamps the layer of the enclosing LayerScope, and
    // within a layer draw order is submission order (see
    // RenderFrame::draw_order).
    frame.submit(std::move(item));
}
} // namespace sif::ui
