/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-15
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETDATASHORT_H
#define RENDER_ENGINE_ASSETDATASHORT_H

#include <string>
#include <unordered_map>

#include "sif/internal/GUID.h"
#include "sif/asset/internal/AssetType.h"


namespace sif::asset::data {
    struct AssetMetaData {
        intrnl::GUID guid;
        AssetType type = AssetType::Undefined;
        std::string asset_name = "NONE";

        /**
         * @brief Promised load time, in seconds.
         *
         * If actually loading the asset takes longer than this, the
         * load is considered late: a warning is logged and the load
         * is retried once with a doubled time budget (see
         * IAssetLoader::try_load). If the retry also exceeds its
         * (doubled) budget, the asset is marked AssetState::Failed.
         */
        double expected_load_time_seconds = 60.0;

        /**
         * @brief Whether this asset is critical.
         *
         * Critical assets jump ahead of non-critical ones in
         * AssetRegistry's load queue (see AssetRegistry::request), so
         * that essential assets (e.g. a loading-screen background or
         * a UI font needed immediately) do not wait behind a long tail
         * of less important ones under a limited concurrency budget.
         */
        bool critical = false;

        std::unordered_map<
            uint32_t,
            std::string
        > record_id_to_name;

        std::unordered_map<
            std::string,
            uint32_t
        > record_name_to_id;
    };
}

#endif //RENDER_ENGINE_ASSETDATASHORT_H