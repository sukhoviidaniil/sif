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
                return static_cast<std::uint8_t>(
                    static_cast<float>(from) + (static_cast<float>(to) - static_cast<float>(from)) * t
                );
            };
            return {lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b), lerp(a.a, b.a)};
        }
    }

    math::Vector2 Button::measure(const math::Vector2 &available) {
        math::Vector2 size{
            std::max(0.f, available.x),
            std::max(0.f, available.y)
        };

        const auto resolve = [](const intrnl::Size& s, const float avail) {
            if (s.type == intrnl::Size::Type::Pixel)   return s.value;
            if (s.type == intrnl::Size::Type::Percent) return avail * s.value;
            return avail;
        };

        if (width.type != intrnl::Size::Type::Auto)  size.x = resolve(width, available.x);
        if (height.type != intrnl::Size::Type::Auto) size.y = resolve(height, available.y);

        size.x = std::clamp(size.x, min_size.x, max_size.x);
        size.y = std::clamp(size.y, min_size.y, max_size.y);

        return size;
    }

    void Button::append_render_items(rnd::RenderFrame &frame, const rnd::FrameContext &ctx) const {
        if (!visible) return;

        auto item = std::make_unique<rnd::Rectangle>();
        item->rect = result.rect;
        item->color = blend(unselected_color, selected_color, transition_);

        frame.constant_items.push_back(std::move(item));

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

    bool Button::contains(const math::Vector2 point) const {
        const intrnl::Rect& r = result.rect;
        return point.x >= r.x && point.x <= r.x + r.width
            && point.y >= r.y && point.y <= r.y + r.height;
    }

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

    void Button::select() {
        state_ = ButtonState::Selected;
    }

    void Button::unselect() {
        state_ = ButtonState::Unselected;
    }

    bool Button::is_selected() const {
        return state_ == ButtonState::Selected;
    }
}
