/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SOUNDNODE_H
#define RENDER_ENGINE_SOUNDNODE_H

#include "AssetDataNode.h"

namespace sif::asset::data {
    /**
     * @brief JSON-facing data for AssetType::Sound.
     *
     * AssetDataNode::source is the audio file to load; no extra
     * fields are needed beyond the base node.
     */
    struct SoundNode : AssetDataNode {

    };
}

#endif //RENDER_ENGINE_SOUNDNODE_H
