/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-13
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_ASSETRECORD_H
#define RENDER_ENGINE_ASSETRECORD_H

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "AssetDesc.h"
#include "AssetState.h"

namespace sif::asset {

    /**
     * @brief Holds the current data and lifecycle state of one asset.
     *
     * Guards against a stale, abandoned load attempt overwriting the
     * result of a newer one: every load dispatch calls begin_attempt()
     * once to obtain a token, and only set_state_if_current() /
     * set_data_if_current() calls that still carry the *current* token
     * are actually applied (see IAssetLoader::try_load). This matters
     * because loads run on detached background threads that are never
     * forcibly cancelled (see IAssetLoader.h) — an old attempt can, in
     * principle, still be running (and finish) after a newer one has
     * already started or concluded.
     *
     * Also supports registering completion callbacks (on_complete) so
     * callers do not have to poll get_state()/get_data() every frame;
     * see AssetHandle<T>::on_ready for the client-facing API.
     */
    struct AssetRecord {

        AssetRecord(AssetRecord&&) noexcept = delete;
        AssetRecord& operator=(AssetRecord&&) noexcept = delete;

        AssetRecord(const AssetRecord&) = delete;
        AssetRecord& operator=(const AssetRecord&) = delete;

        explicit AssetRecord(AssetDesc d);

        AssetRecord(data::AssetMetaData meta, std::string conf_path);

        AssetDesc get_desc() const;
        const data::AssetMetaData& get_meta() const;
        const std::string& get_conf_path() const;

        AssetState get_state() const;

        /**
         * @brief Unconditionally sets the state.
         *
         * Intended for administrative transitions made synchronously by
         * AssetRegistry (e.g. NotRequested -> Queued -> Loading), which
         * are not part of a specific load attempt. If s is a terminal
         * state (Ready or Failed), any pending on_complete callbacks
         * are fired.
         */
        void set_state(AssetState s);

        std::shared_ptr<void> get_data() const;

        /**
         * @brief Unconditionally stores data.
         */
        void set_data(std::shared_ptr<void> d);

        // ========== Attempt guard ==========

        /**
         * @brief Starts a new load attempt and returns its token.
         *
         * Invalidates the token of any previous, still in-flight
         * attempt: once a newer token exists, writes carrying an older
         * token via set_state_if_current() / set_data_if_current() are
         * silently discarded instead of being applied.
         */
        uint64_t begin_attempt();

        /**
         * @brief Returns the token of the most recently started attempt.
         */
        uint64_t current_attempt() const;

        /**
         * @brief Sets the state, but only if attempt_token is still current.
         *
         * If s is a terminal state (Ready or Failed) and the write is
         * applied, any pending on_complete callbacks are fired.
         *
         * @return true if applied, false if discarded as stale (a newer
         * attempt has since started).
         */
        bool set_state_if_current(uint64_t attempt_token, AssetState s);

        /**
         * @brief Stores data, but only if attempt_token is still current.
         *
         * @return true if applied, false if discarded as stale.
         */
        bool set_data_if_current(uint64_t attempt_token, std::shared_ptr<void> d);

        // ========== Completion notification ==========

        using CompletionCallback = std::function<void(AssetState final_state)>;

        /**
         * @brief Registers a callback invoked once the asset reaches a
         * terminal state (Ready or Failed).
         *
         * If the asset has already finished loading, callback is
         * invoked immediately (synchronously, on the calling thread).
         * Otherwise it is invoked later, from whichever thread makes
         * the record reach a terminal state (typically a background
         * loader thread) — keep the callback body short and safe to
         * run from an arbitrary thread.
         *
         * This lets callers react to completion directly instead of
         * polling get_state() every frame.
         */
        void on_complete(CompletionCallback callback);

    private:
        AssetDesc desc;
        mutable std::mutex mtx_;

        AssetState state_ = AssetState::NotRequested;
        std::shared_ptr<void> data_;

        uint64_t attempt_id_ = 0;
        std::vector<CompletionCallback> completion_callbacks_;
    };
} // namespace sif::asset

#endif // RENDER_ENGINE_ASSETRECORD_H