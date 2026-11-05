/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * Description:   A window-less, sound-device-less verification run.
 *                CI machines have no display and no audio device, so
 *                the windowed demos cannot prove anything there. This
 *                one loads every demo asset for real, checks the
 *                decoded results, steps the animation clock, lays out
 *                the layering scene and checks its paint order, and
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

#include "../App.h"
#include "Demo.h"
#include "DemoSupport.h"

#include "sfml/SFMLAssetLoaders.h"

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/AssetImporter.h"
#include "sif/asset/internal/Font.h"
#include "sif/asset/internal/PrimitiveAnimation.h"
#include "sif/asset/internal/Sound.h"
#include "sif/asset/internal/Sprite.h"
#include "sif/asset/internal/SpriteAtlas.h"
#include "sif/asset/internal/SpriteGrid.h"
#include "sif/event/Event_Bus.h"
#include "sif/layout_engine/UIFactory.h"
#include "sif/layout_engine/elements/Overlay.h"
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/render/FrameContext.h"
#include "sif/render/Layer.h"
#include "sif/render/RB_Config.h"
#include "sif/render/RecordingRenderer.h"
#include "sif/render/RenderFrame.h"
#include "sif/render/elements/Rectangle.h"

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

template <typename T>
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
        const intrnl::RecordID by_name = asset::AssetRegistry::instance().record_id_of(guid_of("DemoIcons"), "shield");
        check(by_name == intrnl::RecordID(3), "record name 'shield' resolves to id 3");
    }

    std::cout << "-- SpriteGrid --\n";
    const auto* tiles = load<asset::SpriteGrid>("DemoTiles");
    check(tiles != nullptr, "DemoTiles loaded");
    if (tiles != nullptr) {
        check(tiles->rows() == 2 && tiles->cols() == 4, "DemoTiles is a 2 x 4 grid");
        check(tiles->cell(1, 2) == intrnl::Rect(128.f, 64.f, 64.f, 64.f), "cell (1,2) is tiled at (128, 64, 64, 64)");
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
    for (const auto& [name, expected] :
         std::vector<std::pair<std::string, float>>{{"DemoBlip", 0.18f}, {"DemoChime", 1.10f}, {"DemoThud", 0.32f}}) {
        const auto* sound = load<asset::Sound>(name);
        check(sound != nullptr, name + " decoded");
        if (sound != nullptr) {
            const float d = sound->duration_seconds();
            std::cout << "  " << name << " duration: " << std::fixed << std::setprecision(3) << d << "s\n";
            check(d > expected * 0.9f && d < expected * 1.1f, name + " duration is close to the authored length");
        }
    }

    std::cout << "-- Text (UTF-8) --\n";
    const auto* arial = load<asset::Font>("Arial");
    check(arial != nullptr, "Arial loaded");
    if (arial != nullptr) {
        // Same number of characters, different number of bytes: a
        // font must measure characters, or every non-ASCII label is
        // laid out twice as wide as it is drawn.
        const math::Vector2 latin = arial->measure_text("Pryvit", 20);
        const math::Vector2 cyrillic = arial->measure_text("\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD1\x96\xD1\x82", 20);
        check(latin.x == cyrillic.x, "a 6-letter Cyrillic word measures like a 6-letter Latin one");
    }

    std::cout << "-- Scenes (strict validation) --\n";
    {
        // Every demo scene is built in strict mode, so an attribute a
        // scene uses but its tag's schema does not list (or a typo in a
        // scene) fails CI instead of being silently ignored.
        ui::UIFactory& factory = ui::UIFactory::instance();
        factory.set_strict(true);
        for (const char* scene :
             {"demo_sprites.ui.xml", "demo_animation.ui.xml", "demo_sound.ui.xml", "demo_all.ui.xml",
              "demo_layers.ui.xml", "demo_widgets.ui.xml", "demo_world.ui.xml"}) {
            std::string error;
            try {
                (void)load_scene(scene);
            } catch (const std::exception& e) {
                error = e.what();
            }
            check(error.empty(), std::string(scene) + " builds in strict mode" + (error.empty() ? "" : ": " + error));
        }
        factory.set_strict(false);
    }

    std::cout << "-- Layers (UI layout, no window) --\n";
    {
        // The real scene file the "layers" demo shows, laid out at the
        // demo's window size: overlap, paint order and click routing
        // are checked on production data, not on a hand-built tree.
        const std::unique_ptr<ui::UIElement> root = load_scene("demo_layers.ui.xml");
        check(dynamic_cast<ui::Overlay*>(root.get()) != nullptr, "demo_layers root is an <Overlay>");

        const auto lay_out = [&root] {
            root->measure({1000.f, 640.f});
            root->layout({0.f, 0.f, 1000.f, 640.f});
        };
        lay_out();

        auto* card1 = root->find_by_name<ui::Rectangle>("card1");
        auto* card2 = root->find_by_name<ui::Rectangle>("card2");
        auto* dialog = root->find_by_name<ui::Overlay>("dialog");
        auto* backdrop = root->find_by_name<ui::Rectangle>("dialog_backdrop");
        const ui::UIElement* icon = root->find_by_name("badged_icon");
        const ui::UIElement* corner_badge = root->find_by_name("badge");

        check(card1 && card2 && dialog && backdrop && icon && corner_badge, "every named layering element exists");
        if (card1 && card2 && dialog && backdrop && icon && corner_badge) {
            const intrnl::Rect& a = card1->result.rect;
            const intrnl::Rect& b = card2->result.rect;
            check(a.x + a.width > b.x, "card1 and card2 overlap (negative HBox spacing)");

            const intrnl::Rect& ir = icon->result.rect;
            const intrnl::Rect& br = corner_badge->result.rect;
            check(br.x + br.width > ir.x + ir.width && br.y < ir.y,
                  "the badge sticks out of the icon's top-right corner (Overlay offset)");

            const math::Vector2 overlap{b.x + 1.f, b.y + b.height * 0.5f};
            check(root->element_at(overlap) == card2, "a click on the overlap hits card2 (later sibling on top)");
            card1->z_index = 1;
            check(root->element_at(overlap) == card1, "after z_index = 1, the same click hits card1");

            dialog->visible = true;
            lay_out();
            check(root->element_at(overlap) == backdrop,
                  "with the Modal dialog shown, the click hits its backdrop, not a card");

            rnd::RenderFrame frame;
            const rnd::FrameContext ctx(true);
            root->render(frame, ctx);

            std::size_t card1_at = 0;
            std::size_t backdrop_at = 0;
            const std::vector<const rnd::RenderNode*> order = frame.draw_order();
            for (std::size_t i = 0; i < order.size(); ++i) {
                if (const auto* r = dynamic_cast<const rnd::Rectangle*>(order[i])) {
                    if (r->rect == card1->result.rect)
                        card1_at = i;
                    if (r->rect == backdrop->result.rect && r->layer == rnd::layer::Modal)
                        backdrop_at = i;
                }
            }
            check(backdrop_at > card1_at,
                  "the dialog, declared first in the file, is drawn after the cards (layer=\"Modal\")");
        }
    }

    std::cout << "-- Frame loop (headless backend) --\n";
    {
        // Every windowed demo, driven through App - the same frame loop,
        // factory and demo hooks as on screen - for a few frames on the
        // headless backend (recording renderer, scripted input, silent
        // audio). Catches a demo that throws, or draws nothing, without
        // needing a display.
        const ast::RB_Config config{.type = ast::RB_Type::Headless,
                                    .window_name = "headless",
                                    .window_width = 1000,
                                    .window_height = 640,
                                    .fps = 0};
        const ast::EC_Config input{.type = ast::RB_Type::Headless};

        for (const std::string& id : demo_ids()) {
            std::string error;
            std::size_t draw_calls = 0;
            try {
                App app(std::make_shared<event::Event_Bus>(), config, input, make_demo(id));
                app.run(30);
                if (const auto* recorder = dynamic_cast<const rnd::RecordingRenderer*>(&app.renderer())) {
                    draw_calls = recorder->last_frame().size();
                }
            } catch (const std::exception& e) {
                error = e.what();
            }
            check(error.empty() && draw_calls > 0, "demo '" + id + "' runs 30 frames headless (" +
                                                       std::to_string(draw_calls) + " draw calls)" +
                                                       (error.empty() ? "" : ": " + error));
        }
    }

    std::cout << "-- Asset groups (load a scene, unload it) --\n";
    {
        // A scene's assets are requested while it is built, so capturing
        // the build collects exactly what the scene uses - and releasing
        // the group unloads it again.
        asset::AssetRegistry& registry = asset::AssetRegistry::instance();
        std::unique_ptr<ui::UIElement> scene;
        {
            const asset::AssetRegistry::GroupCapture capture(registry, "headless_world_scene");
            scene = load_scene("demo_world.ui.xml");
        }
        const std::vector<intrnl::GUID> members = registry.group_members("headless_world_scene");
        check(!members.empty(), "building demo_world collected " + std::to_string(members.size()) + " asset(s)");
        registry.request_group("headless_world_scene");
        const bool loaded = registry.wait_for_idle(std::chrono::seconds(10));
        const std::size_t before = registry.loaded_count();
        const std::size_t released = registry.release_group("headless_world_scene");
        check(loaded && released == members.size() && registry.loaded_count() == before - released,
              "release_group unloaded all " + std::to_string(released) + " of them");
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
