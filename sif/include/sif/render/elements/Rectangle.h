/***************************************************************
 * Project:          Render_Engine
 * File:             Rectangle.h
 *
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-24
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_AST_RI_RECTANGLE_H
#define RENDER_ENGINE_AST_RI_RECTANGLE_H

#include "RenderNode.h"
#include "sif/internal/Color.h"

namespace sif::rnd {
    struct Rectangle : RenderNode {
        void accept(RenderVisitor& v) const override { v.visit(*this); }

        int border_width = 0;
        intrnl::Color border_color{};
        intrnl::Color color{};
    };
} // namespace sif::rnd

#endif // RENDER_ENGINE_AST_RI_RECTANGLE_H
