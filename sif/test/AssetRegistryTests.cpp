/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"
#include "Fixtures.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace sif;
using namespace sif::test::fixtures;
using namespace std::chrono_literals;

namespace {
    std::atomic<int> g_concurrent{0};
    std::atomic<int> g_max_observed{0};

    /**
     * @brief Like SleepingLoader, but also tracks how many instances
     * are executing concurrently - needed only by the concurrency-limit
     * test below, so it is local to this file rather than in Fixtures.h.
     */
    class TrackingLoader : public asset::IAssetLoader {
    public:
        explicit TrackingLoader(std::chrono::milliseconds t) : t_(t) {}
    private:
        void load_asset(asset::AssetRecord& record, const std::string&, const uint64_t token) override {
            const int now = ++g_concurrent;
            int prev = g_max_observed.load();
            while (now > prev && !g_max_observed.compare_exchange_weak(prev, now)) {}
            std::this_thread::sleep_for(t_);
            --g_concurrent;
            record.set_data_if_current(token, std::make_shared<int>(1));
        }
        std::chrono::milliseconds t_;
    };

    /**
     * @brief Fails its first load and succeeds every later one - a
     * file that was missing the first time it was asked for.
     */
    class FlakyLoader : public asset::IAssetLoader {
    public:
        std::atomic<int> calls{0};
    private:
        void load_asset(asset::AssetRecord& record, const std::string&, const uint64_t token) override {
            if (calls.fetch_add(1) == 0) {
                throw std::runtime_error("not there yet");
            }
            record.set_data_if_current(token, std::make_shared<int>(7));
        }
    };

    asset::AssetState state_of(const asset::AssetRegistry& reg, const intrnl::GUID id) {
        const auto record = reg.get<int>(id).record().lock();
        return record ? record->get_state() : asset::AssetState::NotRequested;
    }
}

SIF_TEST(asset_registry_default_concurrency_is_two) {
    auto& reg = asset::AssetRegistry::instance();
    SIF_CHECK(reg.max_concurrent_loads() == 2);
}

SIF_TEST(asset_registry_zero_concurrency_clamps_to_one) {
    auto& reg = asset::AssetRegistry::instance();
    const size_t previous = reg.max_concurrent_loads();

    reg.set_max_concurrent_loads(0);
    SIF_CHECK(reg.max_concurrent_loads() == 1);

    reg.set_max_concurrent_loads(previous); // restore for subsequent tests
}

SIF_TEST(asset_registry_respects_concurrency_limit) {
    auto& reg = asset::AssetRegistry::instance();
    reg.set_asset_dir("./");
    reg.set_max_concurrent_loads(2);
    reg.register_loader(asset::AssetType::Font, std::make_unique<TrackingLoader>(80ms));

    g_concurrent = 0;
    g_max_observed = 0;

    std::vector<intrnl::GUID> ids;
    for (int i = 0; i < 6; ++i) {
        ids.push_back(make_test_asset(reg, asset::AssetType::Font));
    }
    for (const auto id : ids) {
        reg.request(id);
    }

    std::this_thread::sleep_for(600ms);

    SIF_CHECK(g_max_observed.load() >= 1);
    SIF_CHECK(g_max_observed.load() <= 2);
}

SIF_TEST(asset_registry_critical_assets_are_dispatched_before_normal_ones) {
    auto& reg = asset::AssetRegistry::instance();
    reg.set_asset_dir("./");
    reg.set_max_concurrent_loads(1); // force strict FIFO-per-queue ordering
    reg.register_loader(asset::AssetType::SpriteSingle, std::make_unique<TrackingLoader>(40ms));

    // Fill the queue with several normal assets first (the first one
    // starts immediately since a slot is free), then request a
    // critical one; because concurrency is 1, the critical asset must
    // sit behind whichever normal asset is already *loading*, but
    // ahead of every other normal asset still only *queued*.
    std::vector<intrnl::GUID> normal_ids;
    for (int i = 0; i < 3; ++i) {
        normal_ids.push_back(make_test_asset(reg, asset::AssetType::SpriteSingle));
    }
    const auto critical_id = make_test_asset(reg, asset::AssetType::SpriteSingle, /*critical=*/true);

    for (const auto id : normal_ids) reg.request(id);
    reg.request(critical_id);

    // With concurrency 1 and 40ms per load: normal_ids[0] starts
    // immediately; critical_id should be next (jumping ahead of
    // normal_ids[1] and [2]); give it enough time to be the 2nd asset
    // to finish, but not enough for all 4 to finish.
    std::this_thread::sleep_for(90ms);

    const bool critical_ready = reg.get<int>(critical_id).ready();
    const bool second_normal_ready = reg.get<int>(normal_ids[1]).ready();

    SIF_CHECK(critical_ready);
    SIF_CHECK(!second_normal_ready);

    std::this_thread::sleep_for(200ms); // let everything settle before the next test
}

SIF_TEST(asset_metadata_defaults_match_specification) {
    asset::data::AssetMetaData meta;
    SIF_CHECK(meta.expected_load_time_seconds == 60.0);
    SIF_CHECK(meta.critical == false);
}

SIF_TEST(attempt_guard_rejects_stale_writes) {
    asset::AssetRecord record(asset::AssetDesc{"conf", asset::data::AssetMetaData{}});

    const uint64_t token1 = record.begin_attempt();
    const uint64_t token2 = record.begin_attempt(); // simulates a newer dispatch starting

    const bool stale_state_applied = record.set_state_if_current(token1, asset::AssetState::Ready);
    const bool stale_data_applied = record.set_data_if_current(token1, std::make_shared<int>(999));
    const bool fresh_state_applied = record.set_state_if_current(token2, asset::AssetState::Failed);

    SIF_CHECK(!stale_state_applied);
    SIF_CHECK(!stale_data_applied);
    SIF_CHECK(fresh_state_applied);
    SIF_CHECK(record.get_state() == asset::AssetState::Failed);
}

SIF_TEST(on_ready_fires_immediately_for_an_already_finished_asset) {
    auto& reg = asset::AssetRegistry::instance();
    reg.set_asset_dir("./");
    reg.register_loader(asset::AssetType::Sound, std::make_unique<SleepingLoader>(1ms));

    const auto id = make_test_asset(reg, asset::AssetType::Sound);
    auto handle = reg.get<int>(id);

    reg.request(id);
    std::this_thread::sleep_for(50ms);

    bool fired_immediately = false;
    int value = 0;
    handle.on_ready([&](int* v) {
        fired_immediately = true;
        value = v ? *v : -1;
    });

    SIF_CHECK(fired_immediately);
    SIF_CHECK(value == 1);
}

SIF_TEST(on_ready_fires_later_for_a_pending_asset) {
    auto& reg = asset::AssetRegistry::instance();
    reg.set_asset_dir("./");
    reg.register_loader(asset::AssetType::SpriteAtlas, std::make_unique<SleepingLoader>(60ms));

    const auto id = make_test_asset(reg, asset::AssetType::SpriteAtlas);
    auto handle = reg.get<int>(id);

    std::atomic<bool> fired{false};
    handle.on_ready([&](int*) { fired = true; });

    SIF_CHECK(!fired.load()); // not requested yet: must not have fired
    reg.request(id);

    std::this_thread::sleep_for(150ms);
    SIF_CHECK(fired.load());
}

SIF_TEST(a_failed_asset_can_be_retried_explicitly) {
    auto& reg = asset::AssetRegistry::instance();
    reg.set_asset_dir("./");
    reg.set_max_concurrent_loads(2);
    auto loader = std::make_unique<FlakyLoader>();
    FlakyLoader* flaky = loader.get();
    reg.register_loader(asset::AssetType::Sound, std::move(loader));

    const auto id = make_test_asset(reg, asset::AssetType::Sound);

    reg.request(id);
    SIF_CHECK(reg.wait_for_idle(5s));
    SIF_CHECK(state_of(reg, id) == asset::AssetState::Failed);

    // request() still never re-loads a failed asset on its own...
    reg.request(id);
    SIF_CHECK(reg.wait_for_idle(5s));
    SIF_CHECK(flaky->calls.load() == 1);

    // ...retry() does.
    SIF_CHECK(reg.retry(id));
    SIF_CHECK(reg.wait_for_idle(5s));
    SIF_CHECK(state_of(reg, id) == asset::AssetState::Ready);
    SIF_CHECK(reg.get<int>(id).ready() && *reg.get<int>(id).get() == 7);
    SIF_CHECK(flaky->calls.load() == 2);

    // Only a Failed asset is retried.
    SIF_CHECK(!reg.retry(id));
    SIF_CHECK(flaky->calls.load() == 2);

    bool threw = false;
    try {
        (void)reg.retry(intrnl::GUID(0xFFFFFFF0u)); // never handed out by make_test_asset
    } catch (const std::runtime_error&) {
        threw = true;
    }
    SIF_CHECK(threw);
}

// =================================================================
// Unloading and asset groups (audit 4.1)
// =================================================================

namespace {
    /// @brief Counts loads; optionally slow, so a load can be caught in flight.
    class CountingLoader : public asset::IAssetLoader {
    public:
        explicit CountingLoader(std::chrono::milliseconds t = 0ms) : t_(t) {}
        std::atomic<int> loads{0};
    private:
        void load_asset(asset::AssetRecord& record, const std::string&, const uint64_t token) override {
            ++loads;
            std::this_thread::sleep_for(t_);
            record.set_data_if_current(token, std::make_shared<int>(42));
        }
        std::chrono::milliseconds t_;
    };

    CountingLoader* install_counting_loader(asset::AssetRegistry& reg, std::chrono::milliseconds t = 0ms) {
        auto loader = std::make_unique<CountingLoader>(t);
        CountingLoader* raw = loader.get();
        reg.register_loader(asset::AssetType::Sound, std::move(loader));
        return raw;
    }
}

SIF_TEST(release_drops_data_and_a_later_request_loads_again) {
    auto& reg = asset::AssetRegistry::instance();
    reg.set_max_concurrent_loads(2);
    CountingLoader* loader = install_counting_loader(reg);
    const auto id = make_test_asset(reg, asset::AssetType::Sound);

    SIF_CHECK(!reg.release(id)); // never requested: nothing to release

    reg.request(id);
    SIF_CHECK(reg.wait_for_idle(5s));
    const auto handle = reg.get<int>(id);
    SIF_CHECK(handle.ready());

    // Someone still holding the data (a playing voice does exactly this)
    // keeps it alive across the release.
    const std::shared_ptr<int> held = handle.lock();

    SIF_CHECK(reg.release(id));
    SIF_CHECK(reg.state_of(id) == asset::AssetState::NotRequested);
    SIF_CHECK(!handle.ready() && handle.get() == nullptr);
    SIF_CHECK(held != nullptr && *held == 42);

    reg.request(id);
    SIF_CHECK(reg.wait_for_idle(5s));
    SIF_CHECK(handle.ready() && loader->loads.load() == 2);

    bool threw = false;
    try { (void)reg.release(intrnl::GUID(0xFFFFFFF1u)); } catch (const std::runtime_error&) { threw = true; }
    SIF_CHECK(threw);
}

SIF_TEST(release_takes_a_queued_asset_off_the_queue_and_discards_a_load_in_flight) {
    auto& reg = asset::AssetRegistry::instance();
    reg.set_max_concurrent_loads(1);
    CountingLoader* loader = install_counting_loader(reg, 80ms);

    const auto loading = make_test_asset(reg, asset::AssetType::Sound);
    const auto queued = make_test_asset(reg, asset::AssetType::Sound);
    reg.request(loading);
    reg.request(queued);
    std::this_thread::sleep_for(20ms);
    SIF_CHECK(reg.state_of(loading) == asset::AssetState::Loading);
    SIF_CHECK(reg.state_of(queued) == asset::AssetState::Queued);

    SIF_CHECK(reg.release(queued));
    SIF_CHECK(reg.release(loading));
    SIF_CHECK(reg.wait_for_idle(5s));

    SIF_CHECK(loader->loads.load() == 1);                                 // the queued one never ran
    SIF_CHECK(reg.state_of(loading) == asset::AssetState::NotRequested); // late result discarded
    SIF_CHECK(!reg.get<int>(loading).ready());
    reg.set_max_concurrent_loads(2);
}

SIF_TEST(release_group_keeps_assets_another_active_group_needs) {
    auto& reg = asset::AssetRegistry::instance();
    reg.set_max_concurrent_loads(4);
    install_counting_loader(reg);

    const auto level_only = make_test_asset(reg, asset::AssetType::Sound);
    const auto shared = make_test_asset(reg, asset::AssetType::Sound);
    const auto menu_only = make_test_asset(reg, asset::AssetType::Sound);
    reg.add_to_group("test_level", level_only);
    reg.add_to_group("test_level", shared);
    reg.add_to_group("test_level", shared); // no duplicates
    reg.add_to_group("test_menu", shared);
    reg.add_to_group("test_menu", menu_only);
    SIF_CHECK(reg.group_members("test_level").size() == 2);

    const std::size_t before = reg.loaded_count();
    reg.request_group("test_level");
    reg.request_group("test_menu");
    SIF_CHECK(reg.wait_for_idle(5s));
    SIF_CHECK(reg.loaded_count() == before + 3);

    SIF_CHECK(reg.release_group("test_level") == 1); // the shared asset stays for the menu
    SIF_CHECK(reg.state_of(level_only) == asset::AssetState::NotRequested);
    SIF_CHECK(reg.state_of(shared) == asset::AssetState::Ready);

    SIF_CHECK(reg.release_group("test_menu") == 2);
    SIF_CHECK(reg.state_of(shared) == asset::AssetState::NotRequested);
    SIF_CHECK(reg.loaded_count() == before);
    SIF_CHECK(reg.release_group("no_such_group") == 0);
}

SIF_TEST(group_capture_collects_everything_requested_inside_it) {
    auto& reg = asset::AssetRegistry::instance();
    install_counting_loader(reg);
    const auto a = make_test_asset(reg, asset::AssetType::Sound);
    const auto b = make_test_asset(reg, asset::AssetType::Sound);
    const auto outside = make_test_asset(reg, asset::AssetType::Sound);
    {
        const asset::AssetRegistry::GroupCapture capture(reg, "test_scene");
        reg.request(a);
        reg.request(b);
    }
    reg.request(outside);
    SIF_CHECK(reg.wait_for_idle(5s));

    const auto members = reg.group_members("test_scene");
    SIF_CHECK(members.size() == 2 && members[0] == a && members[1] == b);
    SIF_CHECK(reg.release_group("test_scene") == 2);
    SIF_CHECK(reg.state_of(outside) == asset::AssetState::Ready);
}
