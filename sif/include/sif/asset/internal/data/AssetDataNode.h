/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-14
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_METAASSETDATA_H
#define RENDER_ENGINE_METAASSETDATA_H

#include <string>

#include "AssetMetaData.h"

namespace sif::asset::data {
    /**
     * @brief Base of every parsed *.asset.json node.
     *
     * Loaders hand these back as std::unique_ptr<AssetDataNode> holding a
     * derived object (FontNode, PrimitiveAnimationNode, ...), so the base
     * needs a virtual destructor: deleting a derived object through a
     * base pointer without one is undefined behaviour, and here it really
     * did misbehave - AddressSanitizer caught the tool freeing 200 bytes
     * of a 232-byte PrimitiveAnimationNode, corrupting the allocator and
     * turning the next unrelated allocation into a crash.
     *
     * The rule of five follows: declaring a destructor suppresses the
     * implicit move operations, so they are restored explicitly rather
     * than silently degrading every node copy into a deep copy.
     */
    struct AssetDataNode {
        virtual ~AssetDataNode() = default;

        AssetDataNode() = default;
        AssetDataNode(const AssetDataNode&) = default;
        AssetDataNode& operator=(const AssetDataNode&) = default;
        AssetDataNode(AssetDataNode&&) = default;
        AssetDataNode& operator=(AssetDataNode&&) = default;

        std::string source;
        AssetMetaData meta;
    };
} // namespace sif::asset::data

#endif // RENDER_ENGINE_METAASSETDATA_H