/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * Description:   The seven windowed demos and the id -> demo registry.
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
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/Font.h"
#include "sif/layout_engine/elements/Animation.h"
#include "sif/layout_engine/elements/Button.h"
#include "sif/layout_engine/elements/Menu.h"
#include "sif/layout_engine/elements/Overlay.h"
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/layout_engine/elements/ScrollView.h"
#include "sif/layout_engine/elements/Sprite.h"
#include "sif/layout_engine/elements/Text.h"
#include "sif/layout_engine/elements/V_HBox.h"
#include "sif/layout_engine/elements/WorldView.h"

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
        if (key == Key::R) { // the on-screen hint always said R; the key enum now has it
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
                  "active voices: " + std::to_string(ctx.audio.active_voices()) +
                      "   master volume: " + std::to_string(static_cast<int>(ctx.audio.master_volume() * 100)) +
                      "%   chime pan " + std::to_string(static_cast<int>(pan_ * 100)) + "%  pitch " +
                      std::to_string(static_cast<int>(pitch_ * 100)) + "%" +
                      (ctx.audio.is_paused(voices_[1]) ? "  (paused)" : ""));
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
        // The looping chime (key 2) is the voice the per-voice controls
        // act on: it lasts long enough to hear a pan, a pitch change or
        // a fade.
        case Key::Space:
            if (ctx.audio.is_paused(voices_[1])) {
                ctx.audio.resume(voices_[1]);
            } else {
                ctx.audio.pause(voices_[1]);
            }
            break;
        case Key::Left:
        case Key::Right:
            pan_ = std::clamp(pan_ + (key == Key::Left ? -0.25f : 0.25f), -1.f, 1.f);
            ctx.audio.set_pan(voices_[1], pan_);
            break;
        case Key::W:
        case Key::S:
            pitch_ = std::clamp(pitch_ * (key == Key::W ? 1.12f : 1.f / 1.12f), 0.5f, 2.f);
            ctx.audio.set_pitch(voices_[1], pitch_);
            break;
        case Key::D:
            ctx.audio.fade_out(voices_[1], 1.5f);
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
    float pan_ = 0.f;
    float pitch_ = 1.f;
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
// 5. Layers and overlapping
// =================================================================

/**
 * @brief Overlapping elements, paint order and click routing.
 *
 * Three things the layout engine could not do before, all driven by
 * the scene file rather than by C++: siblings that overlap (an HBox
 * with negative spacing, and Overlay containers), a sibling brought
 * to the front by z_index, and a dialog declared *first* in the file
 * that still covers everything because it sits on the Modal layer.
 * Clicks are resolved with UIElement::element_at, so the label shows
 * exactly what is on top under the cursor - and that a visible dialog
 * really blocks the cards underneath instead of letting clicks
 * through.
 */
class LayersDemo final : public Demo {
public:
    [[nodiscard]] std::string id() const override { return "layers"; }

    [[nodiscard]] std::string title() const override {
        return "Layers: Overlay + z_index + layer, with clicks routed to whatever is on top";
    }

    [[nodiscard]] std::unique_ptr<ui::UIElement> build_ui() override { return load_scene("demo_layers.ui.xml"); }

    void on_start(Context& ctx) override {
        for (const char* name : {"card1", "card2", "card3"}) {
            cards_.push_back(ctx.root.find_by_name<ui::Rectangle>(name));
        }
        dialog_ = ctx.root.find_by_name<ui::Overlay>("dialog");
    }

    void update(Context& ctx, float /*dt*/) override {
        set_label(ctx.root, "status",
                  "last click: " + last_click_ + "   front card: " + front_card_ +
                      "   dialog: " + (dialog_ != nullptr && dialog_->visible ? "shown" : "hidden"));
    }

    void on_key(Context& /*ctx*/, const Key key) override {
        switch (key) {
        case Key::Num1:
            bring_to_front(0);
            break;
        case Key::Num2:
            bring_to_front(1);
            break;
        case Key::Num3:
            bring_to_front(2);
            break;
        case Key::Space:
            if (dialog_ != nullptr) {
                dialog_->visible = !dialog_->visible;
            }
            break;
        default:
            break;
        }
    }

    void on_click(Context& ctx, const math::Vector2 point) override {
        ui::UIElement* hit = ctx.root.element_at(point);
        if (hit == nullptr) {
            last_click_ = "nothing";
            return;
        }
        last_click_ = hit->name.empty() ? std::string("(unnamed element)") : hit->name;

        // Only the card that is actually on top under the cursor is
        // raised; with the dialog shown, the backdrop is on top and
        // the click never reaches a card.
        for (std::size_t i = 0; i < cards_.size(); ++i) {
            if (cards_[i] != nullptr && cards_[i] == hit) {
                bring_to_front(i);
            }
        }
    }

private:
    void bring_to_front(const std::size_t index) {
        if (index >= cards_.size() || cards_[index] == nullptr) {
            return;
        }
        // A strictly increasing z_index always lands above every
        // sibling raised before it; ties are never needed.
        cards_[index]->z_index = ++top_z_index_;
        front_card_ = cards_[index]->name;
    }

    std::vector<ui::Rectangle*> cards_;
    ui::Overlay* dialog_ = nullptr;
    int top_z_index_ = 0;
    std::string last_click_ = "nothing yet";
    std::string front_card_ = "card3 (last in the file)";
};

// =================================================================
// 6. Widgets: input routing, scrolling, text layout
// =================================================================

/**
 * @brief Buttons, a menu, a scrolling log and wrapped text, all driven
 * by ui::InputRouter.
 *
 * Nothing here hit-tests or tracks focus by hand: App hands raw input to
 * the router, and the demo only installs what each button *does*
 * (Button::on_click) and reads back state for its status line. Mouse
 * and keyboard reach the same code path - a click and Enter on a focused
 * button are the same Click event - which is what the log shows.
 */
class WidgetsDemo final : public Demo {
public:
    [[nodiscard]] std::string id() const override { return "widgets"; }

    [[nodiscard]] std::string title() const override {
        return "Widgets: input routing (hover/press/focus/click), ScrollView, wrapped text";
    }

    [[nodiscard]] std::unique_ptr<ui::UIElement> build_ui() override { return load_scene("demo_widgets.ui.xml"); }

    void on_start(Context& ctx) override {
        font_ = asset::AssetRegistry::instance().get<asset::Font>(guid_of("Arial"));
        menu_ = ctx.root.find_by_name<ui::Menu>("menu");
        log_ = ctx.root.find_by_name<ui::ScrollView>("log");
        log_list_ = ctx.root.find_by_name<ui::VBox>("log_list");

        for (const char* name : {"easy", "normal", "hard", "nightmare"}) {
            if (auto* b = ctx.root.find_by_name<ui::Button>(name)) {
                b->on_click = [this, name](ui::Button&) { log(std::string("difficulty: ") + name); };
            }
        }
        if (auto* counter = ctx.root.find_by_name<ui::Button>("counter")) {
            // The tree outlives every callback (App owns it); ctx itself
            // is a temporary and must not be captured.
            ui::UIElement* root = &ctx.root;
            counter->on_click = [this, root](ui::Button&) {
                ++clicks_;
                set_label(*root, "counter_label", "Clicked " + std::to_string(clicks_) + " times");
                log("counter clicked (" + std::to_string(clicks_) + ")");
            };
        }

        // Start with keyboard focus on the menu, so the arrow keys work
        // straight away.
        if (ctx.input != nullptr) {
            ctx.input->focus(ctx.root.find_by_name<ui::Button>("normal"));
        }
        for (int i = 1; i <= 12; ++i) {
            log("log line " + std::to_string(i) + " - scroll with the wheel");
        }
    }

    void update(Context& ctx, float /*dt*/) override {
        std::string focus = "none";
        if (ctx.input != nullptr && ctx.input->focused() != nullptr) {
            focus = ctx.input->focused()->name;
        }
        std::string active = "-";
        if (menu_ != nullptr && menu_->active_button_id().has_value()) {
            active = std::to_string(menu_->active_button_id()->id);
        }
        std::string scroll = "-";
        if (log_ != nullptr) {
            scroll = std::to_string(static_cast<int>(log_->scroll_offset().y)) + "/" +
                     std::to_string(static_cast<int>(log_->max_scroll().y));
        }
        set_label(ctx.root, "status", "focus: " + focus + "   menu id: " + active + "   log scroll: " + scroll);
    }

private:
    /// @brief Appends one row to the scrolling log and keeps it scrolled to the end.
    void log(const std::string& line) {
        if (log_list_ == nullptr) {
            return;
        }
        auto row = std::make_unique<ui::Overlay>();
        row->height = intrnl::Size::px(26.f);

        auto background = std::make_unique<ui::Rectangle>(
            (log_list_->children.size() % 2 == 0) ? intrnl::Color(40, 40, 54) : intrnl::Color(46, 46, 62));
        background->h_align = intrnl::Align::Stretch;
        background->v_align = intrnl::Align::Stretch;
        row->add(std::move(background));

        auto text = std::make_unique<ui::Text>(font_);
        text->text = line;
        text->fontSize = 16;
        text->color = intrnl::Color(220, 220, 235);
        text->ellipsis = true;
        text->v_text_align = intrnl::Align::Center;
        text->h_align = intrnl::Align::Stretch;
        text->v_align = intrnl::Align::Stretch;
        text->margin = {8.f, 0.f};
        row->add(std::move(text));

        // The ScrollView has follow_end set in the scene, so the new row
        // scrolls into view at the next layout if the log was at its end.
        log_list_->add(std::move(row));
    }

    asset::AssetHandle<asset::Font> font_;
    ui::Menu* menu_ = nullptr;
    ui::ScrollView* log_ = nullptr;
    ui::VBox* log_list_ = nullptr;
    int clicks_ = 0;
};

// =================================================================
// 7. WorldView: the world through a Camera, with a HUD on top
// =================================================================

/**
 * @brief A world in normalized coordinates, shown by a ui::WorldView.
 *
 * Everything in the world - tiles, sprites, the moving ball - is placed
 * in world units (world_rect) and projected by the view's rnd::Camera;
 * the HUD is ordinary UI on the Popup layer. Moving the camera moves
 * the whole world and nothing else, and a click is converted back into
 * world coordinates, which is all a game needs to pick what was clicked.
 */
class WorldDemo final : public Demo {
public:
    [[nodiscard]] std::string id() const override { return "world"; }

    [[nodiscard]] std::string title() const override {
        return "WorldView: world objects projected by a Camera, HUD layered on top";
    }

    [[nodiscard]] std::unique_ptr<ui::UIElement> build_ui() override { return load_scene("demo_world.ui.xml"); }

    void on_start(Context& ctx) override {
        world_ = ctx.root.find_by_name<ui::WorldView>("world");
        ball_ = ctx.root.find_by_name<ui::Animation>("ball");
    }

    void update(Context& ctx, const float dt) override {
        elapsed_ += dt;
        if (ball_ != nullptr) {
            // The ball circles the origin in world units; the camera
            // decides where that is on screen.
            const float radius = 0.45f;
            ball_->world_rect =
                intrnl::Rect(std::cos(elapsed_) * radius - 0.12f, std::sin(elapsed_) * radius - 0.12f, 0.24f, 0.24f);
        }
        if (world_ == nullptr) {
            return;
        }
        std::size_t culled = 0;
        for (const auto& child : world_->children) {
            culled += world_->culled(*child) ? 1u : 0u;
        }
        const math::Point2 c = world_->camera.center();
        set_label(ctx.root, "status",
                  "camera (" + fixed(c.x) + ", " + fixed(c.y) + ")  zoom " + fixed(world_->camera.zoom()) +
                      "   culled " + std::to_string(culled) + "   last click " + last_click_);
    }

    void on_key(Context& /*ctx*/, const Key key) override {
        if (world_ == nullptr) {
            return;
        }
        rnd::Camera& camera = world_->camera;
        const float step = 0.15f / camera.zoom();
        const math::Point2 c = camera.center();
        switch (key) {
        case Key::Left:
            camera.set_center({c.x - step, c.y});
            break;
        case Key::Right:
            camera.set_center({c.x + step, c.y});
            break;
        case Key::Up:
            camera.set_center({c.x, c.y - step});
            break;
        case Key::Down:
            camera.set_center({c.x, c.y + step});
            break;
        case Key::W:
            camera.set_zoom(camera.zoom() * 1.25f);
            break;
        case Key::S:
            camera.set_zoom(camera.zoom() / 1.25f);
            break;
        case Key::D:
            camera.set_center({0.f, 0.f});
            camera.set_zoom(1.f);
            break;
        default:
            break;
        }
    }

    void on_click(Context& /*ctx*/, const math::Vector2 point) override {
        if (world_ != nullptr) {
            const math::Point2 w = world_->to_world(point);
            last_click_ = "(" + fixed(w.x) + ", " + fixed(w.y) + ")";
        }
    }

private:
    static std::string fixed(const float v) {
        char buffer[32];
        std::snprintf(buffer, sizeof buffer, "%.2f", static_cast<double>(v));
        return buffer;
    }

    ui::WorldView* world_ = nullptr;
    ui::Animation* ball_ = nullptr;
    float elapsed_ = 0.f;
    std::string last_click_ = "-";
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

template <typename T>
std::unique_ptr<Demo> create_demo() {
    return std::make_unique<T>();
}

const std::vector<Entry>& entries() {
    static const std::vector<Entry> table = {
        {"sprites", SpritesDemo{}.title(), &create_demo<SpritesDemo>},
        {"animation", AnimationDemo{}.title(), &create_demo<AnimationDemo>},
        {"sound", SoundDemo{}.title(), &create_demo<SoundDemo>},
        {"all", AllDemo{}.title(), &create_demo<AllDemo>},
        {"layers", LayersDemo{}.title(), &create_demo<LayersDemo>},
        {"widgets", WidgetsDemo{}.title(), &create_demo<WidgetsDemo>},
        {"world", WorldDemo{}.title(), &create_demo<WorldDemo>},
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
