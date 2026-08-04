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
