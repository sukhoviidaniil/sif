/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_PRIMITIVEANIMATIONNODE_H
#define RENDER_ENGINE_PRIMITIVEANIMATIONNODE_H

#include <vector>

#include "AssetDataNode.h"
#include "sif/internal/Rect.h"

namespace sif::asset::data {
    /**
     * @brief JSON-facing data for AssetType::PrimitiveAnimation.
     *
     * AssetDataNode::source is one shared image (a strip/sheet); frames[i]
     * is the pixel rect of the i-th animation frame, shown in order for
     * frame_duration_seconds each before advancing to the next (see
     * asset::PrimitiveAnimation::frame_at, which encapsulates this
     * "transition timer" logic so callers do not have to reimplement it).
     */
    struct PrimitiveAnimationNode : AssetDataNode {
        std::vector<intrnl::Rect> frames;

        /**
         * @brief How long each frame is shown, in seconds.
         */
        float frame_duration_seconds = 0.1f;

        /**
         * @brief Whether the animation restarts from frame 0 after the
         * last frame, or holds on the last frame.
         */
        bool loop = true;
    };
}

#endif //RENDER_ENGINE_PRIMITIVEANIMATIONNODE_H
