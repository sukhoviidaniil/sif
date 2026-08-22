/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-11
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#include "sif/layout_engine/elements/UIElement.h"

#include <algorithm>

namespace sif::ui {
    void UIElement::add(std::unique_ptr<UIElement> child) {
        children.push_back(std::move(child));
    }

    math::Vector2 UIElement::measure(const math::Vector2& available) {
        math::Vector2 total_size{0, 0};

        for (auto& child : children) {
            auto child_size = child->measure(available);
            total_size.y += child_size.y + child->margin.y;
            total_size.x = std::max(total_size.x, child_size.x + child->margin.x);
        }

        total_size.x += padding.x * 2;
        total_size.y += padding.y * 2;

        total_size.x = std::clamp(total_size.x, min_size.x, max_size.x);
        total_size.y = std::clamp(total_size.y, min_size.y, max_size.y);

        return total_size;
    }

    void UIElement::layout(const intrnl::Rect r) {
        result.rect = r;

        float y_offset = r.y + padding.y;

        for (auto& child : children) {
            math::Vector2 child_size = child->resolve_size({r.width - padding.x * 2, r.height - padding.y * 2});

            intrnl::Rect child_rect{r.x + padding.x + child->margin.x, y_offset + child->margin.y, child_size.x,
                                    child_size.y};

            child->layout(child_rect);

            y_offset += child_size.y + child->margin.y;
        }
    }

    void UIElement::append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const {
        for (const auto& c : children) {
            c->append_render_items(frame, ctx);
        }
    }

    void UIElement::update(const float dt) {
        for (auto& c : children) {
            c->update(dt);
        }
    }

    UIElement* UIElement::find_if(const std::function<bool(const UIElement&)>& predicate) {
        if (predicate(*this)) {
            return this;
        }
        for (auto& c : children) {
            if (UIElement* found = c->find_if(predicate)) {
                return found;
            }
        }
        return nullptr;
    }

    math::Vector2 UIElement::resolve_size(const math::Vector2& available) const {
        math::Vector2 out;

        auto resolve = [&](const intrnl::Size s, const float avail) {
            if (s.type == intrnl::Size::Type::Pixel)
                return s.value;
            if (s.type == intrnl::Size::Type::Percent)
                return avail * s.value;
            return avail; // Auto
        };

        out.x = resolve(width, available.x);
        out.y = resolve(height, available.y);

        out.x = std::clamp(out.x, min_size.x, max_size.x);
        out.y = std::clamp(out.y, min_size.y, max_size.y);

        return out;
    }
} // namespace sif::ui
