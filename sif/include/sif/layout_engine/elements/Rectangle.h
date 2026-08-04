/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-26
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_RECTANGLE_H
#define RENDER_ENGINE_RECTANGLE_H

#include "sif/math/Vector2.h"
#include "sif/layout_engine/elements/UIElement.h"
#include "sif/internal/Color.h"

namespace sif::ui {

    class Rectangle : public UIElement {
    public:
        explicit Rectangle();
        explicit Rectangle(
            intrnl::Color fill,
            intrnl::Color border = {},
            int border_w = 0
        );

        math::Vector2 measure(const math::Vector2& available) override;

        void append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const override;

        intrnl::Color fill_color;
        intrnl::Color border_color;
        int border_width = 0;

    private:
        static float resolve(const intrnl::Size& s, float avail);
    };

}


#endif //RENDER_ENGINE_RECTANGLE_H