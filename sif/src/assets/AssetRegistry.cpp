/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-13
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include <algorithm>
#include <stdexcept>
#include <thread>

#include "sif/diagnostics/LogScope.h"
#include "sif/diagnostics/Logger.h"

#include "sif/asset/AssetRegistry.h"

namespace sif::asset {
AssetRegistry& AssetRegistry::instance() {
    static AssetRegistry instance;
    return instance;
}

AssetRegistry::AssetRegistry() = default;

bool AssetRegistry::uniq_GUID_locked(const intrnl::GUID& guid) const { return !by_guid_.contains(guid); }

void AssetRegistry::set_asset_dir(const std::string& dir) {
    std::lock_guard lock(mtx_);
    asset_dir = dir;
}

void AssetRegistry::set_max_concurrent_loads(size_t max_concurrent_loads) {
    {
        std::lock_guard lock(mtx_);
        if (max_concurrent_loads == 0) {
            LOG("AssetRegistry::set_max_concurrent_loads - 0 is not allowed, clamping to 1");
            max_concurrent_loads = 1;
        }
        max_concurrent_loads_ = max_concurrent_loads;
    }
    // Raising the limit may free up capacity for already queued assets.
    try_start_next_load();
}

size_t AssetRegistry::max_concurrent_loads() const {
    std::lock_guard lock(mtx_);
    return max_concurrent_loads_;
}

void AssetRegistry::register_loader(const AssetType type, std::unique_ptr<IAssetLoader> loader) {
    std::lock_guard lock(mtx_);
    loaders_[type] = std::move(loader);
}

void AssetRegistry::add_AssetRecord(AssetDesc desc) {
    std::lock_guard lock(mtx_);
    const intrnl::GUID id = desc.meta.guid;
    if (!uniq_GUID_locked(id)) {
        LOG("Double adding GUID: " + id.string());
        return;
    }
    const auto record_ptr = std::make_shared<AssetRecord>(std::move(desc.meta), std::move(desc.conf_path));
    by_guid_[id] = record_ptr;
}

void AssetRegistry::add_AssetRecord(const AssetRecord& other) {
    std::lock_guard lock(mtx_);
    const intrnl::GUID id = other.get_meta().guid;
    if (!uniq_GUID_locked(id)) {
        LOG("Double adding GUID: " + id.string());
        return;
    }
    const auto record_ptr = std::make_shared<AssetRecord>(other.get_desc());
    by_guid_[id] = record_ptr;
}

void AssetRegistry::request(const intrnl::GUID id) {
    LOG_SCOPE();
    LOG("Requested GUID: " + id.string());
    {
        std::lock_guard lock(mtx_);

        const auto rec = by_guid_.find(id);
        if (rec == by_guid_.end()) {
            const std::string err = "Failed to load GUID: " + id.string();
            LOG(err);
            throw std::runtime_error(err);
        }

        const std::shared_ptr<AssetRecord> record = rec->second;

        // Inside a GroupCapture, every requested asset joins the
        // innermost capturing group - also one that was requested before.
        if (!capture_stack_.empty()) {
            add_to_group_locked(capture_stack_.back(), id);
            groups_[capture_stack_.back()].active = true;
        }

        // If we have already requested (queued, loading, ready or
        // failed) it before, there is nothing left to do.
        if (record->get_state() != AssetState::NotRequested) {
            return;
        }

        const AssetType type = record->get_meta().type;
        if (!loaders_.contains(type)) {
            record->set_state(AssetState::Failed);
            const std::string err = "There is no loader for: " + to_string(type);
            LOG(err);
            return;
        }

        // Enqueue the request (critical assets get their own queue,
        // always drained first - see try_start_next_load); the actual
        // background load is started once a slot is free.
        record->set_state(AssetState::Queued);
        if (record->get_meta().critical) {
            critical_queue_.push(id);
        } else {
            normal_queue_.push(id);
        }
    }

    try_start_next_load();
}

bool AssetRegistry::retry(const intrnl::GUID id) {
    LOG_SCOPE();
    {
        std::lock_guard lock(mtx_);

        const auto rec = by_guid_.find(id);
        if (rec == by_guid_.end()) {
            const std::string err = "AssetRegistry::retry - unknown GUID " + id.string();
            LOG(err);
            throw std::runtime_error(err);
        }

        const std::shared_ptr<AssetRecord> record = rec->second;
        if (record->get_state() != AssetState::Failed) {
            return false;
        }

        LOG("Retrying GUID: " + id.string());

        // Back to the start of the life cycle (NotRequested is not a
        // terminal state, so no completion callback fires for it);
        // request() below then treats it as a first request.
        record->set_data(nullptr);
        record->set_state(AssetState::NotRequested);
    }

    // Outside the lock: request() takes mtx_ itself.
    request(id);
    return true;
}

bool AssetRegistry::release_locked(AssetRecord& record) {
    const AssetState state = record.get_state();
    if (state == AssetState::NotRequested) {
        return false;
    }
    if (state == AssetState::Loading) {
        // Invalidate the load in flight: whatever it writes later carries
        // a stale token and is discarded (see AssetRecord::begin_attempt).
        (void)record.begin_attempt();
    }
    // A Queued record is skipped when its turn comes (it is no longer
    // Queued); nothing has to be dug out of the queue.
    record.set_data(nullptr);
    record.set_state(AssetState::NotRequested);
    return true;
}

bool AssetRegistry::release(const intrnl::GUID id) {
    std::lock_guard lock(mtx_);
    const auto rec = by_guid_.find(id);
    if (rec == by_guid_.end()) {
        const std::string err = "AssetRegistry::release - unknown GUID " + id.string();
        LOG(err);
        throw std::runtime_error(err);
    }
    const bool released = release_locked(*rec->second);
    if (released) {
        LOG("Released GUID: " + id.string());
    }
    return released;
}

AssetState AssetRegistry::state_of(const intrnl::GUID id) const {
    std::lock_guard lock(mtx_);
    const auto rec = by_guid_.find(id);
    if (rec == by_guid_.end()) {
        throw std::runtime_error("AssetRegistry::state_of - unknown GUID " + id.string());
    }
    return rec->second->get_state();
}

std::size_t AssetRegistry::loaded_count() const {
    std::lock_guard lock(mtx_);
    std::size_t n = 0;
    for (const auto& [guid, record] : by_guid_) {
        n += record->get_state() == AssetState::Ready ? 1u : 0u;
    }
    return n;
}

void AssetRegistry::add_to_group_locked(const std::string& group, const intrnl::GUID id) {
    std::vector<intrnl::GUID>& members = groups_[group].members;
    if (std::find(members.begin(), members.end(), id) == members.end()) {
        members.push_back(id);
    }
}

void AssetRegistry::add_to_group(const std::string& group, const intrnl::GUID id) {
    std::lock_guard lock(mtx_);
    if (!by_guid_.contains(id)) {
        throw std::runtime_error("AssetRegistry::add_to_group - unknown GUID " + id.string());
    }
    add_to_group_locked(group, id);
}

std::vector<intrnl::GUID> AssetRegistry::group_members(const std::string& group) const {
    std::lock_guard lock(mtx_);
    const auto it = groups_.find(group);
    return it == groups_.end() ? std::vector<intrnl::GUID>{} : it->second.members;
}

void AssetRegistry::request_group(const std::string& group) {
    std::vector<intrnl::GUID> members;
    {
        std::lock_guard lock(mtx_);
        Group& g = groups_[group];
        g.active = true;
        members = g.members;
    }
    // Outside the lock: request() takes it itself.
    for (const intrnl::GUID id : members) {
        request(id);
    }
}

std::size_t AssetRegistry::release_group(const std::string& group) {
    std::lock_guard lock(mtx_);
    const auto it = groups_.find(group);
    if (it == groups_.end()) {
        return 0;
    }
    it->second.active = false;

    std::size_t released = 0;
    for (const intrnl::GUID id : it->second.members) {
        // Still needed by another active group (the menu's font, say)?
        const bool shared = std::any_of(groups_.begin(), groups_.end(), [&](const auto& entry) {
            return entry.second.active && std::find(entry.second.members.begin(), entry.second.members.end(), id) !=
                                              entry.second.members.end();
        });
        if (shared) {
            continue;
        }
        const auto rec = by_guid_.find(id);
        if (rec != by_guid_.end() && release_locked(*rec->second)) {
            ++released;
        }
    }
    LOG("Released group '" + group + "': " + std::to_string(released) + " asset(s)");
    return released;
}

AssetRegistry::GroupCapture::GroupCapture(AssetRegistry& registry, std::string group) : registry_(registry) {
    std::lock_guard lock(registry_.mtx_);
    registry_.groups_[group].active = true;
    registry_.capture_stack_.push_back(std::move(group));
}

AssetRegistry::GroupCapture::~GroupCapture() {
    std::lock_guard lock(registry_.mtx_);
    registry_.capture_stack_.pop_back();
}

data::AssetMetaData AssetRegistry::meta_of(const intrnl::GUID id) const {
    std::lock_guard lock(mtx_);
    const auto rec = by_guid_.find(id);
    if (rec == by_guid_.end()) {
        throw std::runtime_error("AssetRegistry::meta_of - unknown GUID " + id.string());
    }
    return rec->second->get_meta();
}

AssetType AssetRegistry::type_of(const intrnl::GUID id) const { return meta_of(id).type; }

intrnl::RecordID AssetRegistry::record_id_of(const intrnl::GUID id, const std::string& record_name) const {
    const data::AssetMetaData meta = meta_of(id);
    const auto it = meta.record_name_to_id.find(record_name);
    if (it == meta.record_name_to_id.end()) {
        throw std::runtime_error("AssetRegistry::record_id_of - asset " + id.string() + " has no record named '" +
                                 record_name + "'");
    }
    return intrnl::RecordID(it->second);
}

void AssetRegistry::try_start_next_load() {
    std::lock_guard lock(mtx_);

    while (active_loads_ < max_concurrent_loads_ && (!critical_queue_.empty() || !normal_queue_.empty())) {
        // Critical assets always jump the line ahead of normal ones.
        std::queue<intrnl::GUID>& queue = !critical_queue_.empty() ? critical_queue_ : normal_queue_;

        const intrnl::GUID id = queue.front();
        queue.pop();

        const auto rec = by_guid_.find(id);
        if (rec == by_guid_.end()) {
            continue; // record was removed while it was queued
        }

        const std::shared_ptr<AssetRecord> record = rec->second;
        if (record->get_state() != AssetState::Queued) {
            continue; // released (or re-queued and already started) while it waited
        }
        const AssetType type = record->get_meta().type;

        const auto ld = loaders_.find(type);
        if (ld == loaders_.end()) {
            // Loader was unregistered after this asset was queued.
            record->set_state(AssetState::Failed);
            const std::string err = "There is no loader for: " + to_string(type);
            LOG(err);
            continue;
        }

        IAssetLoader* loader = ld->second.get();

        // Some backends cannot be touched from a worker thread at
        // all (see IAssetLoader::runs_on_main_thread). Those are set
        // aside for pump() instead of being given a thread, and they
        // deliberately do not occupy a load slot: they are not in
        // flight, they are waiting for the frame loop.
        if (loader->runs_on_main_thread()) {
            main_thread_queue_.push(id);
            continue;
        }

        record->set_state(AssetState::Loading);
        ++active_loads_;

        std::weak_ptr<asset::AssetRecord> weak_record = record;
        const std::string dir = asset_dir;

        // Use a raw pointer so as not to move unique_ptr
        std::thread t([this, loader, weak_record, dir] {
            if (const auto locked_record = weak_record.lock()) {
                try {
                    loader->try_load(*locked_record, dir);
                } catch (...) {
                    // try_load already logged the error and set
                    // AssetState::Failed; swallow here so a
                    // detached worker thread never propagates an
                    // exception out of the thread function.
                }
            }
            on_load_finished();
        });

        t.detach();
    }
}

void AssetRegistry::on_load_finished() {
    {
        std::lock_guard lock(mtx_);
        if (active_loads_ > 0) {
            --active_loads_;
        }
    }
    idle_cv_.notify_all();
    try_start_next_load();
}

void AssetRegistry::pump(const std::chrono::milliseconds budget) {
    const auto deadline = std::chrono::steady_clock::now() + budget;

    while (std::chrono::steady_clock::now() < deadline) {
        std::shared_ptr<AssetRecord> record;
        IAssetLoader* loader = nullptr;
        std::string dir;

        {
            std::lock_guard lock(mtx_);
            if (main_thread_queue_.empty()) {
                return;
            }

            const intrnl::GUID id = main_thread_queue_.front();
            main_thread_queue_.pop();

            const auto rec = by_guid_.find(id);
            if (rec == by_guid_.end()) {
                continue; // record was removed while it was queued
            }

            record = rec->second;
            if (record->get_state() != AssetState::Queued) {
                record.reset();
                continue; // released while it waited for the frame loop
            }
            const auto ld = loaders_.find(record->get_meta().type);
            if (ld == loaders_.end()) {
                record->set_state(AssetState::Failed);
                LOG("There is no loader for: " + to_string(record->get_meta().type));
                continue;
            }

            loader = ld->second.get();
            dir = asset_dir;
            record->set_state(AssetState::Loading);
        }

        // Outside the lock: a load is slow, and holding mtx_ across
        // it would stall every request() and get() in the program.
        try {
            loader->try_load(*record, dir);
        } catch (...) {
            // try_load already logged and marked the record Failed.
        }

        idle_cv_.notify_all();
    }
}

bool AssetRegistry::wait_for_idle(const std::chrono::milliseconds timeout) const {
    std::unique_lock lock(mtx_);
    // main_thread_queue_ is deliberately not part of the condition:
    // only pump() can drain it, pump() runs on this very thread, and
    // waiting here for work that only we could do is a deadlock.
    return idle_cv_.wait_for(lock, timeout,
                             [this] { return active_loads_ == 0 && critical_queue_.empty() && normal_queue_.empty(); });
}
} // namespace sif::asset