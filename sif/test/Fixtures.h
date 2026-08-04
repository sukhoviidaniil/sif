/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_TEST_FIXTURES_H
#define RENDER_ENGINE_TEST_FIXTURES_H

#include <chrono>
#include <memory>
#include <string>

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/Sprite.h"
#include "sif/asset/internal/SpriteAtlas.h"
#include "sif/asset/internal/SpriteGrid.h"
#include "sif/asset/internal/PrimitiveAnimation.h"
#include "sif/asset/internal/Sound.h"

namespace sif::test::fixtures {
    /**
     * @brief Fake asset::Sprite/SpriteAtlas/SpriteGrid/Sound
     * implementations standing in for real (e.g. SFML-backed) ones.
     *
     * Kept separate from the test files that use them (asset registry
     * tests, sprite tests, etc.) so the same fixtures can be reused by
     * new tests without duplicating fake-loader boilerplate; this is
     * the "separate the test suite from the data being tested" split
     * the test suite as a whole follows.
     */
    struct FakeSprite : asset::Sprite {
        math::Vector2 size() const override;
    };

    struct FakeSpriteAtlas : asset::SpriteAtlas {
        explicit FakeSpriteAtlas(math::Vector<intrnl::Rect> rects);
        math::Vector2 texture_size() const override;
    };

    struct FakeSpriteGrid : asset::SpriteGrid {
        explicit FakeSpriteGrid(math::Matrix<intrnl::Rect> cells);
        math::Vector2 texture_size() const override;
    };

    struct FakePrimitiveAnimation : asset::PrimitiveAnimation {
        FakePrimitiveAnimation(math::Vector<intrnl::Rect> frames, float frame_duration_seconds, bool loop);
        math::Vector2 texture_size() const override;
    };

    struct FakeSound : asset::Sound {
        float duration_seconds() const override;
    };

    /**
     * @brief A loader that sleeps for a fixed duration before
     * succeeding, used to exercise the load queue / concurrency limit.
     */
    class SleepingLoader : public asset::IAssetLoader {
    public:
        explicit SleepingLoader(std::chrono::milliseconds sleep_time);

    private:
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
        std::chrono::milliseconds sleep_time_;
    };

    /**
     * @brief Registers a new AssetRecord with a fresh, unique GUID and
     * returns that GUID, for tests that just need "some asset of type X".
     *
     * @param registry Registry to add the record to.
     * @param type Asset type to assign.
     * @param critical Value for AssetMetaData::critical.
     * @param expected_load_time_seconds Value for AssetMetaData::expected_load_time_seconds.
     */
    intrnl::GUID make_test_asset(
        asset::AssetRegistry& registry,
        asset::AssetType type,
        bool critical = false,
        double expected_load_time_seconds = 10.0
    );

    /**
     * @brief Writes content to path, overwriting any existing file.
     *
     * Used by tests that exercise real JSON/XML parsing against a
     * temporary file on disk.
     */
    void write_file(const std::string& path, const std::string& content);
}

#endif //RENDER_ENGINE_TEST_FIXTURES_H
