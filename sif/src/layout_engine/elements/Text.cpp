/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/Text.h"
#include "sif/render/elements/Text.h"

#include <algorithm>

namespace sif::ui {

    math::Vector2 Text::measure(const math::Vector2& available) {
        if (!font.ready() || text.empty()) {
            return {0.f, 0.f};
        }

        const auto size = static_cast<unsigned int>(std::max(0, fontSize));

        // Ask the font for real metrics instead of guessing
        // "characters * font size": the estimate was wrong for every
        // proportional font, which is most of them.
        math::Vector2 measured = font.get()->measure_text(text, size);

        if (stretch_font) {
            measured.x *= 1.1f;
        }

        // Never claim more room than the parent offered.
        measured.x = std::min(measured.x, available.x);
        measured.y = std::min(std::max(measured.y, static_cast<float>(size)), available.y);

        return measured;
    }

    void Text::append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& /*ctx*/) const {
        if (!font.ready() || text.empty())
            return;
        std::unique_ptr<rnd::Text> item = std::make_unique<rnd::Text>();

        item->rect = result.rect;
        item->font = font;
        item->text = text;
        item->size = fontSize;
        item->color = color;
        // temp_items, not constant_items: the two buckets are a *caching*
        // distinction (rebuilt every frame vs. not), and the renderer
        // happens to draw the constant ones first. Using them as a z-order
        // means every animated entity is painted on top of every label -
        // a pause overlay ends up underneath the bombs it is supposed to
        // cover. Everything that participates in normal layering goes in
        // one bucket, so draw order is submission order.
        frame.temp_items.push_back(std::move(item));
    }
} // namespace sif::ui
