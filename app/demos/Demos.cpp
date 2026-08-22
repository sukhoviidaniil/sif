/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * Description:   The four windowed demos and the id -> demo registry.
 *                Each demo is a handful of lines because everything it
 *                shows is driven by the engine: the scene comes from a
 *                *.ui.xml file, the assets from the registry, and the
 *                frame loop from App.
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "Demo.h"
#include "DemoSupport.h"

#include <algorithm>
#include <string>
#include <vector>

#include "sif/asset/AssetRegistry.h"
#include "sif/layout_engine/elements/Animation.h"
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/layout_engine/elements/Sprite.h"
#include "sif/layout_engine/elements/Text.h"

namespace app::demo {
    using namespace sif;
    using Key = event::input::Key;

    namespace {
        std::string frame_list(const std::vector<ui::Animation*>& cursors) {
            std::string out = "frames:";
            for (const ui::Animation* a : cursors) {
                out += ' ';
                out += std::to_string(a->current_frame());
            }
            return out;
        }
    } // namespace

    // =================================================================
    // 1. Sprites only
    // =================================================================

    /**
     * @brief Shows the three still-image asset types side by side.
     *
     * All three are the same UI element (ui::Sprite) pointed at
     * different assets, which is the actual claim being demonstrated:
     * "whole texture", "named entry of an atlas" and "cell of a grid"
     * differ only in the asset descriptor, not in engine code.
     */
    class SpritesDemo final : public Demo {
    public:
        [[nodiscard]] std::string id() const override { return "sprites"; }

        [[nodiscard]] std::string title() const override {
            return "Static sprites: SpriteSingle + SpriteAtlas (by name) + SpriteGrid (by id)";
        }

        [[nodiscard]] std::unique_ptr<ui::UIElement> build_ui() override { return load_scene("demo_sprites.ui.xml"); }

        void on_start(Context& ctx) override {
            // Typed lookup by name is provided by the engine, so the
            // demo needs no cast of its own.
            badge_ = ctx.root.find_by_name<ui::Sprite>("badge");
        }

        void update(Context& ctx, const float dt) override {
            elapsed_ += dt;
            const bool ready = badge_ != nullptr && badge_->ready();
            set_label(ctx.root, "status",
                      std::string("badge asset: ") + (ready ? "ready" : "loading...") + "   uptime " +
                          std::to_string(static_cast<int>(elapsed_)) + "s");
        }

    private:
        ui::Sprite* badge_ = nullptr;
        float elapsed_ = 0.f;
    };

    // =================================================================
    // 2. Animation only
    // =================================================================

    /**
     * @brief Four independent playback cursors over one animation asset.
     *
     * The point of the demo is that the asset is shared (loaded once,
     * one texture) while the cursors are not: different speeds, and one
     * paused, prove that timing lives in the element and not in the
     * asset.
     */
    class AnimationDemo final : public Demo {
    public:
        [[nodiscard]] std::string id() const override { return "animation"; }

        [[nodiscard]] std::string title() const override {
            return "Animation: one PrimitiveAnimation asset driving four independent cursors";
        }

        [[nodiscard]] std::unique_ptr<ui::UIElement> build_ui() override { return load_scene("demo_animation.ui.xml"); }

        void on_start(Context& ctx) override {
            for (const char* name : {"slow", "normal", "fast", "paused"}) {
                if (auto* a = ctx.root.find_by_name<ui::Animation>(name)) {
                    cursors_.push_back(a);
                }
            }
        }

        void update(Context& ctx, float /*dt*/) override {
            if (!cursors_.empty()) {
                set_label(ctx.root, "frames", frame_list(cursors_));
            }
        }

        void on_key(Context& ctx, const Key key) override {
            if (key == Key::Space) {
                if (auto* paused = ctx.root.find_by_name<ui::Animation>("paused")) {
                    paused->playing() ? paused->pause() : paused->play();
                }
            }
            if (key == Key::S) { // "restart" is bound to S because the engine enum has no 'R'
                for (ui::Animation* a : cursors_) {
                    a->restart();
                }
            }
        }

    private:
        std::vector<ui::Animation*> cursors_;
    };

    // =================================================================
    // 3. Sound only
    // =================================================================

    /**
     * @brief Plays three sound assets on demand, with a visual echo.
     *
     * Sound is invisible, so each key also flashes the matching bar:
     * the bar is driven by the *player's* voice state, not by the key
     * press, so what you see is what is actually still playing.
     */
    class SoundDemo final : public Demo {
    public:
        [[nodiscard]] std::string id() const override { return "sound"; }

        [[nodiscard]] std::string title() const override {
            return "Sound: three Sound assets played through the backend-agnostic AudioPlayer";
        }

        [[nodiscard]] std::unique_ptr<ui::UIElement> build_ui() override { return load_scene("demo_sound.ui.xml"); }

        void on_start(Context& ctx) override {
            blip_ = request_sound("DemoBlip");
            chime_ = request_sound("DemoChime");
            thud_ = request_sound("DemoThud");

            for (const char* name : {"bar1", "bar2", "bar3"}) {
                bars_.push_back(ctx.root.find_by_name<ui::Rectangle>(name));
            }
        }

        void update(Context& ctx, const float dt) override {
            // Announce the startup chime once the asset finishes loading.
            if (!greeted_ && chime_.ready()) {
                greeted_ = true;
                voices_[1] = ctx.audio.play(chime_, 0.6f, false);
                flash_[1] = 1.f;
            }

            for (std::size_t i = 0; i < bars_.size(); ++i) {
                const bool playing = voices_[i] != audio::invalid_voice && ctx.audio.is_playing(voices_[i]);
                if (playing) {
                    flash_[i] = 1.f;
                } else {
                    flash_[i] = std::max(0.f, flash_[i] - dt * 2.f);
                }

                if (bars_[i] != nullptr) {
                    const auto lit = static_cast<std::uint8_t>(60.f + 180.f * flash_[i]);
                    bars_[i]->fill_color = intrnl::Color(lit, static_cast<std::uint8_t>(60 + 40 * i), 90);
                }
            }

            set_label(ctx.root, "voices",
                      "active voices: " + std::to_string(ctx.audio.active_voices()) + "   master volume: " +
                          std::to_string(static_cast<int>(ctx.audio.master_volume() * 100)) + "%");
        }

        void on_key(Context& ctx, const Key key) override {
            switch (key) {
            case Key::Num1:
                voices_[0] = ctx.audio.play(blip_, 0.9f, false);
                break;
            case Key::Num2:
                // Toggling a looping voice is the clearest proof
                // that a VoiceId really identifies one playback.
                if (voices_[1] != audio::invalid_voice && ctx.audio.is_playing(voices_[1])) {
                    ctx.audio.stop(voices_[1]);
                    voices_[1] = audio::invalid_voice;
                } else {
                    voices_[1] = ctx.audio.play(chime_, 0.7f, true);
                }
                break;
            case Key::Num3:
                voices_[2] = ctx.audio.play(thud_, 1.0f, false);
                break;
            case Key::Down:
                ctx.audio.set_master_volume(ctx.audio.master_volume() - 0.1f);
                break;
            case Key::Up:
                ctx.audio.set_master_volume(ctx.audio.master_volume() + 0.1f);
                break;
            default:
                break;
            }
        }

    private:
        asset::AssetHandle<asset::Sound> blip_;
        asset::AssetHandle<asset::Sound> chime_;
        asset::AssetHandle<asset::Sound> thud_;

        std::vector<ui::Rectangle*> bars_;
        audio::VoiceId voices_[3] = {audio::invalid_voice, audio::invalid_voice, audio::invalid_voice};
        float flash_[3] = {0.f, 0.f, 0.f};
        bool greeted_ = false;
    };

    // =================================================================
    // 4. Everything at once
    // =================================================================

    /**
     * @brief Sprites, animation and sound in one scene, driven together.
     *
     * The interesting part is the coupling: a sound is triggered by the
     * *animation's* frame index wrapping back to 0, and the atlas icon
     * is re-pointed at a different record on the same beat. Nothing
     * here polls a timer of its own - the animation cursor is the
     * single clock, which is what makes the three subsystems provably
     * in sync rather than merely running side by side.
     */
    class AllDemo final : public Demo {
    public:
        [[nodiscard]] std::string id() const override { return "all"; }

        [[nodiscard]] std::string title() const override {
            return "Everything together: sprites + animation + sound synchronised by the animation clock";
        }

        [[nodiscard]] std::unique_ptr<ui::UIElement> build_ui() override { return load_scene("demo_all.ui.xml"); }

        void on_start(Context& ctx) override {
            ball_ = ctx.root.find_by_name<ui::Animation>("ball");
            pulse_ = ctx.root.find_by_name<ui::Sprite>("pulse");
            thud_ = request_sound("DemoThud");
            blip_ = request_sound("DemoBlip");
            chime_ = request_sound("DemoChime");

            for (int i = 0; i < 8; ++i) {
                tiles_.push_back(ctx.root.find_by_name<ui::Sprite>("t" + std::to_string(i)));
            }
        }

        void update(Context& ctx, float /*dt*/) override {
            if (ball_ == nullptr) {
                return;
            }

            const size_t frame = ball_->current_frame();

            if (frame != last_frame_) {
                // A loop just wrapped: make a noise and advance the
                // atlas icon, so all three subsystems visibly share one
                // clock.
                if (frame < last_frame_) {
                    ++loops_;
                    ctx.audio.play(thud_, 0.8f, false);
                    if (pulse_ != nullptr) {
                        static const char* icons[] = {"heart", "gem", "coin", "shield"};
                        pulse_->set_record_id(
                            asset::AssetRegistry::instance().record_id_of(guid_of("DemoIcons"), icons[loops_ % 4]));
                    }
                } else {
                    ctx.audio.play(blip_, 0.18f, false);
                }
                last_frame_ = frame;
            }

            // The tile strip lights up in step with the current frame.
            for (std::size_t i = 0; i < tiles_.size(); ++i) {
                if (tiles_[i] == nullptr) {
                    continue;
                }
                const bool lit = (i == frame % tiles_.size());
                tiles_[i]->tint = lit ? intrnl::Color(255, 255, 255) : intrnl::Color(90, 90, 110);
            }

            set_label(ctx.root, "beat",
                      "frame " + std::to_string(frame) + "   loops " + std::to_string(loops_) + "   voices " +
                          std::to_string(ctx.audio.active_voices()));
        }

        void on_key(Context& ctx, const Key key) override {
            switch (key) {
            case Key::Space:
                if (ball_ != nullptr) {
                    ball_->playing() ? ball_->pause() : ball_->play();
                }
                break;
            case Key::Num1:
                ctx.audio.play(blip_, 0.9f, false);
                break;
            case Key::Num2:
                ctx.audio.play(thud_, 0.9f, false);
                break;
            // Was ctx.audio.stop_all() - the on-screen hint promises
            // "1/2/3 - play a sound manually", matching SoundDemo's own
            // three-key pattern (blip/chime/thud), but this demo only
            // ever loaded two sounds, so pressing 3 silently stopped
            // everything instead of playing anything. DemoChime is the
            // third sound SoundDemo already uses for exactly this slot.
            case Key::Num3:
                ctx.audio.play(chime_, 0.7f, false);
                break;
            default:
                break;
            }
        }

    private:
        ui::Animation* ball_ = nullptr;
        ui::Sprite* pulse_ = nullptr;
        std::vector<ui::Sprite*> tiles_;
        asset::AssetHandle<asset::Sound> thud_;
        asset::AssetHandle<asset::Sound> blip_;
        asset::AssetHandle<asset::Sound> chime_;
        size_t last_frame_ = 0;
        unsigned int loops_ = 0;
    };

    // =================================================================
    // Registry
    // =================================================================

    namespace {
        struct Entry {
            std::string id;
            std::string title;
            std::unique_ptr<Demo> (*create)();
        };

        template<typename T>
        std::unique_ptr<Demo> create_demo() {
            return std::make_unique<T>();
        }

        const std::vector<Entry>& entries() {
            static const std::vector<Entry> table = {
                {"sprites", SpritesDemo{}.title(), &create_demo<SpritesDemo>},
                {"animation", AnimationDemo{}.title(), &create_demo<AnimationDemo>},
                {"sound", SoundDemo{}.title(), &create_demo<SoundDemo>},
                {"all", AllDemo{}.title(), &create_demo<AllDemo>},
            };
            return table;
        }
    } // namespace

    std::vector<std::string> demo_ids() {
        std::vector<std::string> ids;
        ids.reserve(entries().size());
        for (const Entry& e : entries()) {
            ids.push_back(e.id);
        }
        return ids;
    }

    std::string demo_title(const std::string& id) {
        for (const Entry& e : entries()) {
            if (e.id == id) {
                return e.title;
            }
        }
        return {};
    }

    std::unique_ptr<Demo> make_demo(const std::string& id) {
        for (const Entry& e : entries()) {
            if (e.id == id) {
                return e.create();
            }
        }
        return nullptr;
    }
} // namespace app::demo
