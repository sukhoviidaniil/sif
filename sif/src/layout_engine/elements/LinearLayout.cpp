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

#include <algorithm>

namespace sif::ui {
namespace {
/// @brief Component of v along the main axis (x for a row, y for a column).
float main_of(const math::Vector2& v, const bool is_h) { return is_h ? v.x : v.y; }

/// @brief Component of v along the cross axis.
float cross_of(const math::Vector2& v, const bool is_h) { return is_h ? v.y : v.x; }
} // namespace

math::Vector2 LinearLayout::measure(const math::Vector2& available) {
    const bool is_h = horizontal();

    // padding used to be parsed for VBox/HBox/Menu but never applied:
    // padding="16" on a root VBox had no visible effect. Children now
    // only get what is left inside it, and the layout reports it back
    // as part of its own size.
    const math::Vector2 inner{std::max(0.f, available.x - padding.x * 2), std::max(0.f, available.y - padding.y * 2)};

    float main = 0.f;
    float cross = 0.f;
    int visible_count = 0;

    for (auto& c : children) {
        if (!c->visible)
            continue;

        visible_count++;

        // Limit measurement along the main axis to what is still
        // free, and on both axes to what is left inside the child's
        // own margins.
        math::Vector2 limit{std::max(0.f, inner.x - c->margin.x * 2), std::max(0.f, inner.y - c->margin.y * 2)};
        if (is_h)
            limit.x = std::max(0.f, limit.x - main);
        else
            limit.y = std::max(0.f, limit.y - main);

        const math::Vector2 cs = c->measure(limit);

        main += main_of(cs, is_h) + main_of(c->margin, is_h) * 2;
        cross = std::max(cross, cross_of(cs, is_h) + cross_of(c->margin, is_h) * 2);
    }

    if (visible_count > 1) {
        main += spacing * float(visible_count - 1);
    }

    // Never ask for more main-axis space than was offered. An Auto
    // leaf measures as "everything available", so a row of them used
    // to report the whole width *plus* every spacing, and the parent
    // then gave the row more room than the screen has (the tile strip
    // of the "all" demo ran off the right edge). Children that really
    // do not fit still overflow inside the row - layout() places them
    // exactly as before - but the row itself no longer pushes its
    // parent and siblings out of place.
    main = std::min(main, is_h ? inner.x : inner.y);

    math::Vector2 size = is_h ? math::Vector2{main, cross} : math::Vector2{cross, main};
    size.x += padding.x * 2;
    size.y += padding.y * 2;

    // An explicit width/height (PX/PERCENT) on the layout itself wins
    // over its content size on that axis, as it already did for every
    // leaf element; Auto keeps the content size.
    if (width.type != intrnl::Size::Type::Auto)
        size.x = width.resolve(available.x);
    if (height.type != intrnl::Size::Type::Auto)
        size.y = height.resolve(available.y);

    size.x = std::clamp(size.x, min_size.x, max_size.x);
    size.y = std::clamp(size.y, min_size.y, max_size.y);

    return size;
}

void LinearLayout::layout(intrnl::Rect r) {
    result.rect = r;

    const bool is_h = horizontal();

    // Content box: the assigned rect minus padding on every side.
    const intrnl::Rect inner{r.x + padding.x, r.y + padding.y, std::max(0.f, r.width - padding.x * 2),
                             std::max(0.f, r.height - padding.y * 2)};
    const math::Vector2 inner_size{inner.width, inner.height};

    const float main = is_h ? inner.width : inner.height;
    const float cross = is_h ? inner.height : inner.width;

    float fixed = 0.f;
    float flex_sum = 0.f;
    size_t visible_count = 0;

    // Preliminary measurement of fixed children. Margins are fixed
    // space for every child, flexible ones included: flex shares out
    // what is left *after* them.
    for (auto& c : children) {
        if (!c->visible)
            continue;
        ++visible_count;

        fixed += main_of(c->margin, is_h) * 2;

        if (c->flex > 0.f) {
            flex_sum += c->flex;
        } else {
            const math::Vector2 limit{std::max(0.f, inner_size.x - c->margin.x * 2),
                                      std::max(0.f, inner_size.y - c->margin.y * 2)};
            fixed += main_of(c->measure(limit), is_h);
        }
    }

    const float spacing_sum = visible_count > 1 ? spacing * float(visible_count - 1) : 0.f;

    const float free = std::max(0.f, main - fixed - spacing_sum);

    // Where the leftover space goes (see intrnl::Justify). With a
    // flex child present the flex share already took all of it, so
    // justify only matters for layouts made of fixed-size children.
    float lead = 0.f;
    float gap = 0.f;
    if (flex_sum <= 0.f && free > 0.f && visible_count > 0) {
        const auto n = static_cast<float>(visible_count);
        switch (justify) {
        case intrnl::Justify::Center:
            lead = free * 0.5f;
            break;
        case intrnl::Justify::End:
            lead = free;
            break;
        case intrnl::Justify::SpaceBetween:
            gap = visible_count > 1 ? free / (n - 1.f) : 0.f;
            break;
        case intrnl::Justify::SpaceAround:
            gap = free / n;
            lead = gap * 0.5f;
            break;
        case intrnl::Justify::SpaceEvenly:
            gap = free / (n + 1.f);
            lead = gap;
            break;
        case intrnl::Justify::Start:
        default:
            break;
        }
    }

    float cursor = (is_h ? inner.x : inner.y) + lead;

    for (auto& c : children) {
        if (!c->visible)
            continue;

        const float margin_main = main_of(c->margin, is_h);
        const float margin_cross = cross_of(c->margin, is_h);

        const math::Vector2 limit{std::max(0.f, inner_size.x - c->margin.x * 2),
                                  std::max(0.f, inner_size.y - c->margin.y * 2)};
        const math::Vector2 s = c->measure(limit);

        float main_size = (c->flex > 0.f && flex_sum > 0.f) ? free * (c->flex / flex_sum) : main_of(s, is_h);

        main_size = std::max(0.f, main_size);

        // The cross axis is aligned inside the child's own margins.
        const float cross_extent = std::max(0.f, cross - margin_cross * 2);

        float cross_size = (align == intrnl::Align::Stretch) ? cross_extent : cross_of(s, is_h);

        cross_size = std::max(0.f, cross_size);

        float cross_pos = (is_h ? inner.y : inner.x) + margin_cross;

        if (align != intrnl::Align::Stretch) {
            const float extra = cross_extent - cross_size;
            if (align == intrnl::Align::Center)
                cross_pos += extra * 0.5f;
            else if (align == intrnl::Align::End)
                cross_pos += extra;
        }

        const float main_pos = cursor + margin_main;

        intrnl::Rect cr = is_h ? intrnl::Rect{main_pos, cross_pos, main_size, cross_size}
                               : intrnl::Rect{cross_pos, main_pos, cross_size, main_size};

        c->layout(cr);
        cursor += main_size + margin_main * 2 + spacing + gap;
    }
}
} // namespace sif::ui