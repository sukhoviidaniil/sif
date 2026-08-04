/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-14
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SPRITEGRIDNODE_H
#define RENDER_ENGINE_SPRITEGRIDNODE_H

#include <cstdint>

#include "AssetDataNode.h"

namespace sif::asset::data {
    /**
     * @brief JSON-facing data for AssetType::SpriteGrid.
     *
     * AssetDataNode::source is one shared image, cut into a uniform
     * rows x cols grid of equally sized cells. At load time the
     * loader tiles the texture into a math::Matrix<intrnl::Rect> handed
     * to asset::SpriteGrid (see sif/asset/internal/SpriteGrid.h).
     */
    struct SpriteGridNode : AssetDataNode {
        uint32_t rows = 1; ///< Number of grid rows
        uint32_t cols = 1; ///< Number of grid columns
        float cell_width = 0.f; ///< Width of one cell, in pixels
        float cell_height = 0.f; ///< Height of one cell, in pixels
    };
}

#endif //RENDER_ENGINE_SPRITEGRIDNODE_H
