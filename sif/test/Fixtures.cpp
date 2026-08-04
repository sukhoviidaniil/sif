/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "Fixtures.h"

#include <atomic>
#include <fstream>
#include <thread>

namespace sif::test::fixtures {
    math::Vector2 FakeSprite::size() const {
        return {64.f, 64.f};
    }

    FakeSpriteAtlas::FakeSpriteAtlas(math::Vector<intrnl::Rect> rects)
        : asset::SpriteAtlas(std::move(rects)) {
    }

    math::Vector2 FakeSpriteAtlas::texture_size() const {
        return {256.f, 256.f};
    }

    FakeSpriteGrid::FakeSpriteGrid(math::Matrix<intrnl::Rect> cells)
        : asset::SpriteGrid(std::move(cells)) {
    }

    math::Vector2 FakeSpriteGrid::texture_size() const {
        return {128.f, 128.f};
    }

    FakePrimitiveAnimation::FakePrimitiveAnimation(math::Vector<intrnl::Rect> frames, const float frame_duration_seconds, const bool loop)
        : asset::PrimitiveAnimation(std::move(frames), frame_duration_seconds, loop) {
    }

    math::Vector2 FakePrimitiveAnimation::texture_size() const {
        return {100.f, 100.f};
    }

    float FakeSound::duration_seconds() const {
        return 2.f;
    }

    SleepingLoader::SleepingLoader(const std::chrono::milliseconds sleep_time)
        : sleep_time_(sleep_time) {
    }

    void SleepingLoader::load_asset(asset::AssetRecord& record, const std::string&, const uint64_t attempt_token) {
        std::this_thread::sleep_for(sleep_time_);
        record.set_data_if_current(attempt_token, std::make_shared<int>(1));
    }

    namespace {
        std::atomic<uint32_t> g_next_guid{1};
    }

    intrnl::GUID make_test_asset(
        asset::AssetRegistry& registry,
        const asset::AssetType type,
        const bool critical,
        const double expected_load_time_seconds
    ) {
        asset::data::AssetMetaData meta;
        meta.guid = intrnl::GUID(g_next_guid++);
        meta.type = type;
        meta.asset_name = "test_asset_" + meta.guid.string();
        meta.critical = critical;
        meta.expected_load_time_seconds = expected_load_time_seconds;

        asset::AssetDesc desc("unused_conf_path.json", meta);
        registry.add_AssetRecord(std::move(desc));
        return meta.guid;
    }

    void write_file(const std::string& path, const std::string& content) {
        std::ofstream out(path);
        out << content;
    }
}
