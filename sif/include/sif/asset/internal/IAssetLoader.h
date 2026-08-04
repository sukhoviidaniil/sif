/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-14
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_IASSETLOADER_H
#define RENDER_ENGINE_IASSETLOADER_H

#include "AssetRecord.h"

namespace sif::asset {
    class IAssetLoader {
    public:
        virtual ~IAssetLoader();

        /**
         * @brief Loads an asset, enforcing its promised load time.
         *
         * Calls load_asset() and times how long it takes. The asset's
         * metadata carries a promised load time in seconds
         * (AssetMetaData::expected_load_time_seconds). If the load
         * takes longer than that:
         *   - a warning is logged, and
         *   - the asset is reloaded once more with a doubled time budget.
         * If that second attempt also exceeds its (doubled) budget, the
         * asset is marked AssetState::Failed and an error is logged.
         *
         * On an actual loading error (load_asset throwing), the asset
         * is immediately marked AssetState::Failed; there is no retry
         * in that case, only for late-but-successful loads.
         *
         * Loads run on detached background threads that are never
         * forcibly cancelled, so try_load() obtains a fresh attempt
         * token (AssetRecord::begin_attempt()) before starting, and
         * writes the final Ready/Failed state (and load_asset() writes
         * its data) only if that token is still current. This means an
         * old, still-running attempt can never overwrite the result of
         * a newer one that was dispatched for the same asset.
         *
         * @param record Record to populate; also tracks load state.
         * @param asset_dir Root directory assets are loaded from.
         */
        void try_load(AssetRecord& record, const std::string& asset_dir);
    private:

        /**
         * @brief Performs the actual load; must be implemented by subclasses.
         *
         * Implementations should store the loaded data with
         * record.set_data_if_current(attempt_token, data) rather than
         * the unguarded set_data(), and should not set the record's
         * state themselves: try_load() owns the Ready/Failed
         * transition.
         *
         * @param record Record to populate.
         * @param asset_dir Root directory assets are loaded from.
         * @param attempt_token Token from the begin_attempt() call that
         * started this load; pass it to set_data_if_current().
         */
        virtual void load_asset(AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) = 0;
    };
}

#endif //RENDER_ENGINE_IASSETLOADER_H