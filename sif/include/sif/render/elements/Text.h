/***************************************************************
 * Project:          Render_Engine
 * File:            Text.h
 *
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2025-12-24
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_AST_RI_TEXT_H
#define RENDER_ENGINE_AST_RI_TEXT_H

#include <string>

#include "RenderNode.h"

#include "sif/asset/AssetHandle.h"
#include "sif/asset/internal/Font.h"
#include "sif/internal/Color.h"

namespace sif::rnd {
    struct Text : RenderNode {
        void accept(RenderVisitor& v) const override { v.visit(*this); }

        asset::AssetHandle<asset::Font> font;
        intrnl::Color color;
        int size = 0;
        /**
         * @brief Any text
         */
        std::string text;
    };
} // namespace sif::rnd

#endif // RENDER_ENGINE_AST_RI_TEXT_H