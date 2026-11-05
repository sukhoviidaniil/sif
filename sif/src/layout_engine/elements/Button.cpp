/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-07-06
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/Button.h"
#include "sif/render/elements/Rectangle.h"

#include <algorithm>
#include <cstdint>

namespace sif::ui {
namespace {
/**
 * @brief Linearly blends two colors by t in [0, 1].
 */
intrnl::Color blend(const intrnl::Color& a, const intrnl::Color& b, const float t) {
    const auto lerp = [t](const std::uint8_t from, const std::uint8_t to) {
        return static_cast<std::uint8_t>(static_cast<float>(from) +
                                         (static_cast<float>(to) - static_cast<float>(from)) * t);
    };
    return {lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b), lerp(a.a, b.a)};
}

/// @brief Moves each channel of c a fraction t of the way towards `to` (alpha kept).
intrnl::Color shade(const intrnl::Color& c, const std::uint8_t to, const float t) {
    const auto mix = [to, t](const std::uint8_t from) {
        return static_cast<std::uint8_t>(static_cast<float>(from) +
                                         (static_cast<float>(to) - static_cast<float>(from)) * t);
    };
    return {mix(c.r), mix(c.g), mix(c.b), c.a};
}
} // namespace

Button::Button() { focusable = true; }

math::Vector2 Button::measure(const math::Vector2& available) {
    math::Vector2 size{std::max(0.f, available.x), std::max(0.f, available.y)};

    if (width.type != intrnl::Size::Type::Auto)
        size.x = width.resolve(available.x);
    if (height.type != intrnl::Size::Type::Auto)
        size.y = height.resolve(available.y);

    size.x = std::clamp(size.x, min_size.x, max_size.x);
    size.y = std::clamp(size.y, min_size.y, max_size.y);

    return size;
}

void Button::append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const {
    if (!visible)
        return;

    auto item = std::make_unique<rnd::Rectangle>();
    item->rect = result.rect;
    intrnl::Color color = blend(unselected_color, selected_color, transition_);
    if (!enabled) {
        color = disabled_color;
    } else if (pressed) {
        color = shade(color, 0, 0.25f);
    } else if (hovered) {
        color = shade(color, 255, 0.2f);
    }
    item->color = color;
    if (focused) {
        item->border_width = 2;
        item->border_color = focus_color;
    }

    // submit(), not constant_items: the two buckets are a *caching*
    // distinction (rebuilt every frame vs. not), not a z-order. Using
    // them as one meant every animated entity was painted on top of
    // every label - a pause overlay ended up underneath the bombs it
    // was supposed to cover. Covering is now expressed explicitly:
    // submit() stamps the layer of the enclosing LayerScope, and
    // within a layer draw order is submission order (see
    // RenderFrame::draw_order).
    frame.submit(std::move(item));

    // Buttons are leaf elements as far as rendering goes, but still
    // let any (unusual) children render themselves.
    UIElement::append_render_items(frame, ctx);
}

void Button::update(const float dt) {
    const float target = state_ == ButtonState::Selected ? 1.f : 0.f;

    if (transition_duration_seconds <= 0.f) {
        transition_ = target;
    } else if (transition_ < target) {
        transition_ = std::min(target, transition_ + dt / transition_duration_seconds);
    } else if (transition_ > target) {
        transition_ = std::max(target, transition_ - dt / transition_duration_seconds);
    }

    UIElement::update(dt);
}

bool Button::contains(const math::Vector2 point) const { return result.rect.contains(point.x, point.y); }

bool Button::handle_click(const math::Vector2 point) {
    if (!contains(point)) {
        return false;
    }
    if (state_ == ButtonState::Selected) {
        unselect();
    } else {
        select();
    }
    return true;
}

void Button::activate() {
    if (enabled && on_click) {
        on_click(*this);
    }
}

bool Button::on_ui_event(UIEvent& e) {
    if (e.type == UIEventType::Click && enabled) {
        activate();
        e.handled = true;
    }
    return UIElement::on_ui_event(e);
}

bool Button::can_focus() const { return focusable && enabled; }

void Button::select() { state_ = ButtonState::Selected; }

void Button::unselect() { state_ = ButtonState::Unselected; }

bool Button::is_selected() const { return state_ == ButtonState::Selected; }
} // namespace sif::ui
