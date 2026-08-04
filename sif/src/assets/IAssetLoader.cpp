/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-02-01
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#include "sif/asset/internal/IAssetLoader.h"

#include <chrono>

#include "sif/diagnostics/Logger.h"

namespace sif::asset {
    IAssetLoader::~IAssetLoader() = default;

    void IAssetLoader::try_load(AssetRecord &record, const std::string &asset_dir) {
        const std::string& asset_name = record.get_meta().asset_name;

        // Every dispatch gets its own token; only writes carrying this
        // exact token are applied to the record (see AssetRecord.h).
        const uint64_t attempt_token = record.begin_attempt();

        // The asset promises to be loaded within this many seconds. If
        // an attempt takes longer, it is retried once with a doubled
        // budget; exceeding that doubled budget as well is an error.
        double allowed_seconds = record.get_meta().expected_load_time_seconds;
        constexpr int max_attempts = 2;

        for (int attempt = 1; attempt <= max_attempts; ++attempt) {
            LOG("Start loading asset: " + asset_name +
                " (attempt " + std::to_string(attempt) +
                ", budget " + std::to_string(allowed_seconds) + "s)");

            const auto start = std::chrono::steady_clock::now();
            try {
                load_asset(record, asset_dir, attempt_token);
            } catch (...) {
                if (!record.set_state_if_current(attempt_token, AssetState::Failed)) {
                    LOG("Discarded stale failure for '" + asset_name +
                        "': a newer load attempt has since started.");
                }
                const std::string err = "Failed to load asset: " + asset_name;
                std::throw_with_nested(
                    std::runtime_error(err)
                );
            }
            const auto end = std::chrono::steady_clock::now();
            const double elapsed_seconds = std::chrono::duration<double>(end - start).count();

            if (elapsed_seconds <= allowed_seconds) {
                if (record.set_state_if_current(attempt_token, AssetState::Ready)) {
                    LOG("Successfully loaded asset: " + asset_name +
                        " in " + std::to_string(elapsed_seconds) + "s");
                } else {
                    LOG("Discarded stale success for '" + asset_name +
                        "': a newer load attempt has since started.");
                }
                return;
            }

            // The load succeeded, but broke its promised time budget.
            const std::string warning =
                "Asset '" + asset_name + "' exceeded its promised load time (" +
                std::to_string(elapsed_seconds) + "s > " + std::to_string(allowed_seconds) + "s).";
            LOG(warning);

            if (attempt >= max_attempts) {
                const std::string err =
                    "Asset '" + asset_name + "' failed to load within its doubled time budget (" +
                    std::to_string(allowed_seconds) + "s).";
                if (record.set_state_if_current(attempt_token, AssetState::Failed)) {
                    LOG(err);
                } else {
                    LOG("Discarded stale failure for '" + asset_name +
                        "': a newer load attempt has since started.");
                }
                throw std::runtime_error(err);
            }

            allowed_seconds *= 2.0;
            LOG("Retrying asset '" + asset_name + "' with doubled time budget: " +
                std::to_string(allowed_seconds) + "s");
        }
    }
}
