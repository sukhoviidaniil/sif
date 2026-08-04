/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-17
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETDESC_H
#define RENDER_ENGINE_ASSETDESC_H

#include <string>
#include <utility>

#include "data/AssetMetaData.h"

namespace sif::asset {
    /**
     * @brief Data Transfer Object carrying an asset's static description
     * (where its config file lives, plus its metadata) between layers
     * that should not otherwise depend on each other: JSON on disk ->
     * AssetImporter -> AssetRegistry -> AssetRecord. AssetDesc itself
     * has no behavior beyond construction; it exists purely to move
     * this data around as a single value.
     */
    struct AssetDesc {
        AssetDesc() = default;
        AssetDesc(std::string c, data::AssetMetaData m) : conf_path(std::move(c)), meta(std::move(m)) {

        }

        std::string conf_path;
        data::AssetMetaData meta;
    };
}

#endif //RENDER_ENGINE_ASSETDESC_H