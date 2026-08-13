/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-13
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETREGISTRY_H
#define RENDER_ENGINE_ASSETREGISTRY_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <unordered_map>

#include "internal/AssetRecord.h"
#include "internal/AssetType.h"
#include "sif/internal/RecordID.h"
#include "internal/IAssetLoader.h"
#include "AssetHandle.h"


namespace sif::asset {
    /**
     * @brief Central registry of known assets and their load state.
     *
     * Singleton pattern (see instance()): every asset in the program
     * needs to be requested/looked up through the same registry, so
     * a single shared instance (rather than one per subsystem) is the
     * whole point - two registries would mean two independent load
     * queues and two copies of "is this asset ready yet".
     *
     * Asset loads go through two FIFO queues, one for critical assets
     * (AssetMetaData::critical) and one for everything else: request()
     * enqueues the asset into the appropriate queue (if it is not
     * already queued/loading/loaded), and up to max_concurrent_loads()
     * requests are worked on at the same time, each on its own
     * background thread. The critical queue is always drained first,
     * so a critical asset never waits behind a long tail of
     * non-critical ones. As soon as a load finishes (successfully or
     * not), the next queued request (if any) is started.
     */
    class AssetRegistry{
    public:
        static AssetRegistry& instance();

        AssetRegistry(const AssetRegistry&) = delete;
        AssetRegistry& operator=(const AssetRegistry&) = delete;
        AssetRegistry(AssetRegistry&&) = delete;
        AssetRegistry& operator=(AssetRegistry&&) = delete;

        void set_asset_dir(const std::string& dir);

        /**
         * @brief Sets how many assets may be loaded at the same time.
         *
         * Requesting 0 is clamped to 1 (a value of 0 would stall the
         * queue forever) and logged as a warning. If raising the limit
         * frees up capacity, queued requests are started immediately.
         *
         * @param max_concurrent_loads Maximum number of simultaneous loads.
         */
        void set_max_concurrent_loads(size_t max_concurrent_loads);

        /**
         * @brief Returns the current maximum number of simultaneous loads.
         */
        [[nodiscard]] size_t max_concurrent_loads() const;

        void register_loader(
            asset::AssetType type,
            std::unique_ptr<asset::IAssetLoader> loader
            );

        void add_AssetRecord(asset::AssetDesc desc);
        void add_AssetRecord(const asset::AssetRecord& other);

        /**
         * @brief Requests that the asset with the given id be loaded.
         *
         * If the asset has not been requested before, it is placed at
         * the back of the load queue matching its criticality (state
         * becomes AssetState::Queued) and a background load is started
         * as soon as a load slot is free (see set_max_concurrent_loads).
         * Calling this again for an asset that is already queued,
         * loading, ready, or failed has no effect.
         */
        void request(intrnl::GUID id);

        /**
         * @brief Returns a copy of the metadata registered for an asset.
         *
         * @throws std::runtime_error if no asset with that GUID exists.
         */
        [[nodiscard]] data::AssetMetaData meta_of(intrnl::GUID id) const;

        /**
         * @brief Returns the declared AssetType of an asset.
         *
         * Needed by callers that hold a type-erased handle and must
         * know which concrete asset class is behind it - e.g. the UI
         * factory building a <Sprite> from a GUID alone, which has to
         * decide between SpriteSingle/SpriteAtlas/SpriteGrid.
         *
         * @throws std::runtime_error if no asset with that GUID exists.
         */
        [[nodiscard]] AssetType type_of(intrnl::GUID id) const;

        /**
         * @brief Resolves a human-readable record name to its id, using
         * AssetMetaData::record_name_to_id.
         *
         * @throws std::runtime_error if the asset or the name is unknown.
         */
        [[nodiscard]] intrnl::RecordID record_id_of(intrnl::GUID id, const std::string& record_name) const;

        template<typename T>
        AssetHandle<T> get(intrnl::GUID id) const;

        /**
         * @brief Blocks until no load is queued or in flight.
         *
         * Loads run on *detached* threads, so without this a program
         * that exits while an asset is still loading leaves a thread
         * writing into a registry that is being destroyed. Call it
         * before shutting down (App::run does, on the way out).
         *
         * @param timeout Maximum time to wait; returns false if the
         * queue was still busy when it expired.
         */
        bool wait_for_idle(std::chrono::milliseconds timeout = std::chrono::seconds(30)) const;

        /**
         * @brief Runs the loads that must not happen on a worker thread.
         *
         * Call once per frame from the thread that owns the window and
         * the audio device. Loaders that report runs_on_main_thread()
         * are queued rather than dispatched, and this is where they are
         * executed - see IAssetLoader::runs_on_main_thread for why some
         * have to be.
         *
         * Doing nothing when there is nothing queued is the common case,
         * so calling it every frame is cheap. Not calling it at all
         * leaves those assets permanently Queued, which shows up as a
         * sound that never plays rather than as a crash.
         *
         * @param budget Stop after this long and finish the rest next
         * frame, so a burst of queued loads cannot stall a frame.
         */
        void pump(std::chrono::milliseconds budget = std::chrono::milliseconds(4));

    private:
        AssetRegistry();

        /// @brief Caller must hold mtx_.
        bool uniq_GUID_locked(const intrnl::GUID& guid) const;

        /**
         * @brief Starts background loads for queued assets while there
         * is spare capacity (active_loads_ < max_concurrent_loads_).
         *
         * Drains critical_queue_ before normal_queue_, so critical
         * assets are always dispatched first. Safe to call any time;
         * it is a no-op if both queues are empty or the registry is
         * already at capacity.
         */
        void try_start_next_load();

        /**
         * @brief Called by a worker thread once its load attempt is
         * over (successful or not). Frees up a load slot and tries to
         * start the next queued load.
         */
        void on_load_finished();

        std::string asset_dir;

        std::unordered_map<
            intrnl::GUID,
            std::shared_ptr<asset::AssetRecord>,
            intrnl::GUIDHash
        >
        by_guid_;

        std::unordered_map<
            asset::AssetType,
            std::unique_ptr<asset::IAssetLoader>
        > loaders_;

        // ========== Load queue / concurrency control ==========

        /**
         * @brief Guards *all* mutable registry state: by_guid_,
         * loaders_, asset_dir, both queues, active_loads_ and
         * max_concurrent_loads_.
         *
         * One mutex rather than one per field, because background
         * loader threads reach into by_guid_/loaders_ from
         * try_start_next_load() while the main thread may still be
         * calling request()/add_AssetRecord()/get() - guarding only the
         * queues (as an earlier version did) left the maps racing.
         * Never held while a loader callback or a load itself runs.
         */
        mutable std::mutex mtx_;
        std::queue<intrnl::GUID> critical_queue_; ///< Critical assets waiting for a free load slot
        std::queue<intrnl::GUID> normal_queue_;  ///< Non-critical assets waiting for a free load slot

        /// Loads that pump() must run; see IAssetLoader::runs_on_main_thread.
        std::queue<intrnl::GUID> main_thread_queue_;
        size_t active_loads_ = 0; ///< Number of loads currently in flight
        size_t max_concurrent_loads_ = 2; ///< Configurable cap on simultaneous loads
        mutable std::condition_variable idle_cv_; ///< Signalled whenever a load finishes
    };
}

#include "AssetRegistry.inl"

#endif //RENDER_ENGINE_ASSETREGISTRY_H