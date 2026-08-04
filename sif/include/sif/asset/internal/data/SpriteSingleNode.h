/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-14
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SPRITESINGLENODE_H
#define RENDER_ENGINE_SPRITESINGLENODE_H

#include "AssetDataNode.h"

namespace sif::asset::data {
    /**
     * @brief JSON-facing data for AssetType::SpriteSingle.
     *
     * The whole image referenced by AssetDataNode::source is the
     * sprite; no extra fields are needed beyond the base node.
     */
    struct SpriteSingleNode : AssetDataNode {

    };
}

#endif //RENDER_ENGINE_SPRITESINGLENODE_H
