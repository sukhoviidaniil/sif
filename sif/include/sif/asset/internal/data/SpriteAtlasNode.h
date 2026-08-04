/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-14
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SPRITEATLASNODE_H
#define RENDER_ENGINE_SPRITEATLASNODE_H

#include <vector>

#include "AssetDataNode.h"
#include "sif/internal/Rect.h"

namespace sif::asset::data {
    /**
     * @brief JSON-facing data for AssetType::SpriteAtlas.
     *
     * AssetDataNode::source is one shared image, cut into an arbitrary
     * (not necessarily uniform) list of named sub-sprites. rects[i] is
     * the pixel rect of the sub-sprite whose record id is i; optional
     * human-readable names for each id live in
     * AssetMetaData::record_id_to_name (already present on the base
     * meta struct).
     *
     * At load time this list is handed to asset::SpriteAtlas as a
     * math::Vector<intrnl::Rect> (see sif/asset/internal/SpriteAtlas.h).
     */
    struct SpriteAtlasNode : AssetDataNode {
        std::vector<intrnl::Rect> rects;
    };
}

#endif //RENDER_ENGINE_SPRITEATLASNODE_H
