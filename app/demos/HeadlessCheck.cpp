/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * Description:   A window-less, sound-device-less verification run.
 *                CI machines have no display and no audio device, so
 *                the windowed demos cannot prove anything there. This
 *                one loads every demo asset for real, checks the
 *                decoded results and steps the animation clock, and
 *                returns a non-zero exit code on any failure - which
 *                makes "the assets and the timing still work" a
 *                testable claim rather than a screenshot.
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "HeadlessCheck.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "DemoSupport.h"

#include "sfml/SFMLAssetLoaders.h"

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/AssetImporter.h"
#include "sif/asset/internal/PrimitiveAnimation.h"
#include "sif/asset/internal/Sound.h"
#include "sif/asset/internal/Sprite.h"
#include "sif/asset/internal/SpriteAtlas.h"
#include "sif/asset/internal/SpriteGrid.h"

namespace app::demo {
    using namespace sif;

    namespace {
        int failures = 0;

        void check(const bool condition, const std::string& what) {
            std::cout << (condition ? "  [PASS] " : "  [FAIL] ") << what << '\n';
            if (!condition) {
                ++failures;
            }
        }

        /**
         * @brief Blocks until an asset reaches a terminal state.
         *
         * Loads happen on background threads; a headless check has no
         * frame loop to spread them over, so it waits explicitly rather
         * than sleeping a fixed "probably enough" amount.
         */
        bool wait_ready(const intrnl::GUID guid, const std::chrono::milliseconds timeout = std::chrono::seconds(10)) {
            asset::AssetRegistry& registry = asset::AssetRegistry::instance();
            registry.request(guid);

            const auto deadline = std::chrono::steady_clock::now() + timeout;
            const asset::AssetHandle<void> handle = registry.get<void>(guid);

            while (std::chrono::steady_clock::now() < deadline) {
                if (handle.ready()) {
                    return true;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            return handle.ready();
        }

        template<typename T>
        const T* load(const std::string& asset_name) {
            const intrnl::GUID guid = guid_of(asset_name);
            if (!wait_ready(guid)) {
                return nullptr;
            }
            return asset::AssetRegistry::instance().get<T>(guid).get();
        }
    } // namespace

    int run_headless_check() {
        failures = 0;

        std::cout << "== headless verification (no window, no audio device) ==\n";
        std::cout << "data dir: " << Paths::data_dir() << "\n\n";

        std::cout << "-- SpriteSingle --\n";
        const auto* badge = load<asset::Sprite>("DemoBadge");
        check(badge != nullptr, "DemoBadge loaded");
        if (badge != nullptr) {
            check(badge->size().x == 128.f && badge->size().y == 128.f,
                  "DemoBadge texture is 128x128 (got " + std::to_string(static_cast<int>(badge->size().x)) + "x" +
                      std::to_string(static_cast<int>(badge->size().y)) + ")");
        }

        std::cout << "-- SpriteAtlas --\n";
        const auto* icons = load<asset::SpriteAtlas>("DemoIcons");
        check(icons != nullptr, "DemoIcons loaded");
        if (icons != nullptr) {
            check(icons->count() == 4, "DemoIcons has 4 sub-sprites");
            check(icons->rect(intrnl::RecordID(2)).x == 128.f, "record 2 ('coin') starts at x = 128");
            const intrnl::RecordID by_name =
                asset::AssetRegistry::instance().record_id_of(guid_of("DemoIcons"), "shield");
            check(by_name == intrnl::RecordID(3), "record name 'shield' resolves to id 3");
        }

        std::cout << "-- SpriteGrid --\n";
        const auto* tiles = load<asset::SpriteGrid>("DemoTiles");
        check(tiles != nullptr, "DemoTiles loaded");
        if (tiles != nullptr) {
            check(tiles->rows() == 2 && tiles->cols() == 4, "DemoTiles is a 2 x 4 grid");
            check(tiles->cell(1, 2) == intrnl::Rect(128.f, 64.f, 64.f, 64.f),
                  "cell (1,2) is tiled at (128, 64, 64, 64)");
            check(tiles->cell(intrnl::RecordID(6)) == tiles->cell(1, 2),
                  "flat record id 6 addresses the same cell as (1,2)");
        }

        std::cout << "-- PrimitiveAnimation --\n";
        const auto* ball = load<asset::PrimitiveAnimation>("DemoBall");
        check(ball != nullptr, "DemoBall loaded");
        if (ball != nullptr) {
            check(ball->frame_count() == 8, "DemoBall has 8 frames");
            check(ball->loops(), "DemoBall loops");

            // Step the very same clock the UI element uses, so this is
            // a check of production behaviour and not of a copy of it.
            const float step = ball->frame_duration_seconds();
            std::vector<size_t> observed;
            for (int i = 0; i < 10; ++i) {
                observed.push_back(ball->frame_index_at(step * static_cast<float>(i) + step * 0.5f));
            }

            std::string seq;
            for (const size_t f : observed) {
                seq += std::to_string(f);
                seq += ' ';
            }
            std::cout << "  frames over 10 steps: " << seq << '\n';

            check(observed.front() == 0, "playback starts at frame 0");
            check(observed[7] == 7, "frame 7 is reached after 7 steps");
            check(observed[8] == 0, "playback wraps back to frame 0 (loop)");
        }

        std::cout << "-- Sound --\n";
        for (const auto& [name, expected] : std::vector<std::pair<std::string, float>>{
                 {"DemoBlip", 0.18f}, {"DemoChime", 1.10f}, {"DemoThud", 0.32f}}) {
            const auto* sound = load<asset::Sound>(name);
            check(sound != nullptr, name + " decoded");
            if (sound != nullptr) {
                const float d = sound->duration_seconds();
                std::cout << "  " << name << " duration: " << std::fixed << std::setprecision(3) << d << "s\n";
                check(d > expected * 0.9f && d < expected * 1.1f, name + " duration is close to the authored length");
            }
        }

        std::cout << '\n';
        if (failures == 0) {
            std::cout << "HEADLESS CHECK PASSED\n";
            return 0;
        }
        std::cout << "HEADLESS CHECK FAILED (" << failures << " check(s))\n";
        return 1;
    }
} // namespace app::demo
