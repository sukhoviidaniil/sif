/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/LinearLayout.h"

namespace sif::ui {
    math::Vector2 LinearLayout::measure(const math::Vector2& available) {
        const bool is_h = horizontal();

        math::Vector2 total{0.f, 0.f};
        int visible_count = 0;

        for (auto& c : children) {
            if (!c->visible)
                continue;

            visible_count++;

            // Limit measurement along the main axis
            math::Vector2 limit = available;
            if (is_h)
                limit.x = std::max(0.f, available.x - total.x);
            else
                limit.y = std::max(0.f, available.y - total.y);

            math::Vector2 cs = c->measure(limit);

            if (is_h) {
                total.x += cs.x;
                total.y = std::max(total.y, cs.y);
            } else {
                total.y += cs.y;
                total.x = std::max(total.x, cs.x);
            }
        }

        if (visible_count > 1) {
            float sp = spacing * float(visible_count - 1);
            if (is_h)
                total.x += sp;
            else
                total.y += sp;
        }

        return total;
    }

    void LinearLayout::layout(intrnl::Rect r) {
        result.rect = r;

        const bool is_h = horizontal();

        math::Point2 origin{r.x, r.y};
        math::Vector2 size{r.width, r.height};

        const float main = is_h ? size.x : size.y;
        const float cross = is_h ? size.y : size.x;

        float fixed = 0.f;
        float flex_sum = 0.f;
        size_t visible_count = 0;

        // Preliminary measurement of fixed children
        for (auto& c : children) {
            if (!c->visible)
                continue;
            ++visible_count;

            if (c->flex > 0.f) {
                flex_sum += c->flex;
            } else {
                math::Vector2 s = c->measure(size);
                fixed += is_h ? s.x : s.y;
            }
        }

        const float spacing_sum = visible_count > 1 ? spacing * float(visible_count - 1) : 0.f;

        const float free = std::max(0.f, main - fixed - spacing_sum);

        float cursor = is_h ? origin.x : origin.y;

        for (auto& c : children) {
            if (!c->visible)
                continue;

            math::Vector2 s = c->measure(size);

            float main_size = (c->flex > 0.f && flex_sum > 0.f) ? free * (c->flex / flex_sum) : (is_h ? s.x : s.y);

            main_size = std::max(0.f, main_size);

            float cross_size = (align == intrnl::Align::Stretch) ? cross : (is_h ? s.y : s.x);

            cross_size = std::max(0.f, cross_size);

            float cross_pos = is_h ? origin.y : origin.x;

            if (align != intrnl::Align::Stretch) {
                const float extra = cross - cross_size;
                if (align == intrnl::Align::Center)
                    cross_pos += extra * 0.5f;
                else if (align == intrnl::Align::End)
                    cross_pos += extra;
            }

            intrnl::Rect cr = is_h ? intrnl::Rect{cursor, cross_pos, main_size, cross_size}
                                   : intrnl::Rect{cross_pos, cursor, cross_size, main_size};

            c->layout(cr);
            cursor += main_size + spacing;
        }
    }
} // namespace sif::ui