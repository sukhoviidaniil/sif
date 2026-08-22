/***************************************************************
 * Project:          Render_Engine
 * File:            RenderFrame.h
 *
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2025-12-24
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_AST_RENDERFRAME_H
#define RENDER_ENGINE_AST_RENDERFRAME_H

#include <memory>
#include <vector>

#include "elements/RenderNode.h"

namespace sif::rnd {
    struct RenderFrame {
        std::vector<std::unique_ptr<RenderNode>> constant_items;
        std::vector<std::unique_ptr<RenderNode>> temp_items;
    };
} // namespace sif::rnd

#endif // RENDER_ENGINE_AST_RENDERFRAME_H