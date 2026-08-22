/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-02-01
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/asset/internal/AssetRecord.h"

namespace sif::asset {
    namespace {
        bool is_terminal(const AssetState s) {
            return s == AssetState::Ready || s == AssetState::Failed;
        }
    } // namespace

    AssetRecord::AssetRecord(AssetDesc d) : desc(std::move(d)) {}

    AssetRecord::AssetRecord(data::AssetMetaData meta, std::string conf_path)
        : AssetRecord(AssetDesc{std::move(conf_path), std::move(meta)}) {}

    AssetDesc AssetRecord::get_desc() const {
        return desc;
    }

    const data::AssetMetaData& AssetRecord::get_meta() const {
        return desc.meta;
    }

    const std::string& AssetRecord::get_conf_path() const {
        return desc.conf_path;
    }

    AssetState AssetRecord::get_state() const {
        std::lock_guard lock(mtx_);
        return state_;
    }

    void AssetRecord::set_state(AssetState s) {
        std::vector<CompletionCallback> callbacks_to_fire;
        {
            std::lock_guard lock(mtx_);
            state_ = s;
            if (is_terminal(s)) {
                callbacks_to_fire = std::move(completion_callbacks_);
                completion_callbacks_.clear();
            }
        }
        // Invoke outside the lock: a callback may call back into this
        // AssetRecord (e.g. register another on_complete, read state).
        for (const auto& callback : callbacks_to_fire) {
            callback(s);
        }
    }

    std::shared_ptr<void> AssetRecord::get_data() const {
        std::lock_guard lock(mtx_);
        return data_;
    }

    void AssetRecord::set_data(std::shared_ptr<void> d) {
        std::lock_guard lock(mtx_);
        data_ = std::move(d);
    }

    uint64_t AssetRecord::begin_attempt() {
        std::lock_guard lock(mtx_);
        return ++attempt_id_;
    }

    uint64_t AssetRecord::current_attempt() const {
        std::lock_guard lock(mtx_);
        return attempt_id_;
    }

    bool AssetRecord::set_state_if_current(uint64_t attempt_token, AssetState s) {
        std::vector<CompletionCallback> callbacks_to_fire;
        {
            std::lock_guard lock(mtx_);
            if (attempt_token != attempt_id_) {
                return false; // a newer attempt has since started; discard as stale
            }
            state_ = s;
            if (is_terminal(s)) {
                callbacks_to_fire = std::move(completion_callbacks_);
                completion_callbacks_.clear();
            }
        }
        for (const auto& callback : callbacks_to_fire) {
            callback(s);
        }
        return true;
    }

    bool AssetRecord::set_data_if_current(uint64_t attempt_token, std::shared_ptr<void> d) {
        std::lock_guard lock(mtx_);
        if (attempt_token != attempt_id_) {
            return false; // a newer attempt has since started; discard as stale
        }
        data_ = std::move(d);
        return true;
    }

    void AssetRecord::on_complete(CompletionCallback callback) {
        AssetState snapshot;
        {
            std::lock_guard lock(mtx_);
            snapshot = state_;
            if (!is_terminal(snapshot)) {
                completion_callbacks_.push_back(std::move(callback));
                return;
            }
        }
        // Already finished: fire immediately, outside the lock.
        callback(snapshot);
    }
} // namespace sif::asset
