/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-11-05
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"
#include "Fixtures.h"

#include <chrono>
#include <cmath>
#include <memory>

#include "sif/asset/AssetRegistry.h"
#include "sif/audio/Null_AudioPlayer.h"
#include "sif/event/Event_Bus.h"
#include "sif/event/Scripted_Event_Collector.h"
#include "sif/event/events/input_Keyboard.h"
#include "sif/event/events/input_Mouse.h"
#include "sif/event/events/window.hpp"
#include "sif/layout_engine/elements/Overlay.h"
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/render/FrameContext.h"
#include "sif/render/Layer.h"
#include "sif/render/RecordingRenderer.h"

using namespace sif;
using namespace std::chrono_literals;

SIF_TEST(recording_renderer_records_what_would_be_drawn_in_draw_order) {
    ui::Overlay root;
    auto dialog = std::make_unique<ui::Rectangle>(intrnl::Color(9, 0, 0));
    dialog->layer = rnd::layer::Modal;
    root.add(std::move(dialog));
    root.add(std::make_unique<ui::Rectangle>(intrnl::Color(1, 0, 0)));
    root.measure({100.f, 50.f});
    root.layout({0.f, 0.f, 100.f, 50.f});

    rnd::RenderFrame frame;
    root.render(frame, rnd::FrameContext(true));

    rnd::RecordingRenderer renderer({320.f, 200.f});
    renderer.render(frame);

    const auto& calls = renderer.last_frame();
    SIF_CHECK(calls.size() == 2);
    if (calls.size() == 2) {
        SIF_CHECK(calls[0].kind == rnd::DrawCall::Kind::Rectangle && calls[0].color.r == 1);
        SIF_CHECK(calls[1].color.r == 9 && calls[1].layer == rnd::layer::Modal); // declared first, drawn last
        SIF_CHECK(calls[1].rect == intrnl::Rect(0.f, 0.f, 100.f, 50.f));
    }
    SIF_CHECK(renderer.frames_rendered() == 1);

    renderer.render(rnd::RenderFrame{});
    SIF_CHECK(renderer.last_frame().empty()); // one frame at a time
    SIF_CHECK(renderer.frames_rendered() == 2);
}

SIF_TEST(recording_renderer_follows_window_events) {
    const auto bus = std::make_shared<event::Event_Bus>();
    rnd::RecordingRenderer renderer({800.f, 600.f});
    renderer.track_global(bus);

    bus->emit(event::window::Window_Resized{1024, 768});
    SIF_CHECK(renderer.screen_size().x == 1024.f && renderer.screen_size().y == 768.f);

    SIF_CHECK(renderer.is_open());
    bus->emit(event::window::Window_Closed{});
    SIF_CHECK(!renderer.is_open());
}

SIF_TEST(scripted_collector_delivers_queued_events_in_order_on_collect) {
    event::Scripted_Event_Collector collector;
    collector.queue(event::input::KeyPressed{event::input::Key::Space, false, false, false, false});
    collector.queue(event::input::Mouse_ButtonPressed{event::input::MouseButton::Left, 10, 20});
    SIF_CHECK(collector.pending() == 2);
    SIF_CHECK(collector.event_store_.empty()); // nothing until the frame collects

    collector.collect();
    SIF_CHECK(collector.pending() == 0);
    SIF_CHECK(collector.event_store_.size() == 2);
    const auto first = collector.event_store_.pop_concept();
    SIF_CHECK(first->type() == std::type_index(typeid(event::input::KeyPressed)));
    const auto second = collector.event_store_.pop<event::input::Mouse_ButtonPressed>();
    SIF_CHECK(second.x == 10 && second.y == 20);
}

SIF_TEST(null_audio_player_keeps_voices_for_exactly_their_duration) {
    asset::AssetRegistry& registry = asset::AssetRegistry::instance();
    const intrnl::GUID guid = test::fixtures::make_test_asset(registry, asset::AssetType::Sound);
    const auto record = registry.get<void>(guid).record().lock();
    if (record) {
        const std::shared_ptr<asset::Sound> sound = std::make_shared<test::fixtures::FakeSound>(); // 2 s
        record->set_data(std::static_pointer_cast<void>(sound));
        record->set_state(asset::AssetState::Ready);
    }
    const auto handle = registry.get<asset::Sound>(guid);

    auto now = std::chrono::steady_clock::time_point{};
    audio::Null_AudioPlayer player(2, [&now] { return now; });

    const audio::VoiceId once = player.play(handle, 1.f, false);
    const audio::VoiceId looped = player.play(handle, 1.f, true);
    SIF_CHECK(once != audio::invalid_voice && looped != audio::invalid_voice && once != looped);
    SIF_CHECK(player.play(handle) == audio::invalid_voice); // voice limit of 2

    now += 1900ms;
    SIF_CHECK(player.is_playing(once));
    now += 200ms;
    SIF_CHECK(!player.is_playing(once)); // 2 s sound is over
    SIF_CHECK(player.is_playing(looped));
    SIF_CHECK(player.active_voices() == 1);

    player.stop(looped);
    SIF_CHECK(player.active_voices() == 0);

    player.set_master_volume(3.f);
    SIF_CHECK(player.master_volume() == 1.f);

    SIF_CHECK(player.play(asset::AssetHandle<asset::Sound>{}) == audio::invalid_voice); // not loaded
}

namespace {
    /// @brief A ready 2-second FakeSound handle.
    asset::AssetHandle<asset::Sound> two_second_sound() {
        asset::AssetRegistry& registry = asset::AssetRegistry::instance();
        const intrnl::GUID guid = test::fixtures::make_test_asset(registry, asset::AssetType::Sound);
        const auto record = registry.get<void>(guid).record().lock();
        if (record) {
            const std::shared_ptr<asset::Sound> sound = std::make_shared<test::fixtures::FakeSound>();
            record->set_data(std::static_pointer_cast<void>(sound));
            record->set_state(asset::AssetState::Ready);
        }
        return registry.get<asset::Sound>(guid);
    }

    bool near(const float a, const float b) { return std::abs(a - b) < 1e-4f; }
}

SIF_TEST(audio_gain_is_voice_times_bus_times_master) {
    auto now = std::chrono::steady_clock::time_point{};
    audio::Null_AudioPlayer player(8, [&now] { return now; });
    const auto sound = two_second_sound();

    audio::PlayOptions music;
    music.volume = 0.5f;
    music.bus = audio::Bus::Music;
    music.loop = true;
    const audio::VoiceId song = player.play(sound, music);
    const audio::VoiceId blip = player.play(sound, 0.8f, false); // the original signature: Effects bus

    player.set_bus_volume(audio::Bus::Music, 0.5f);
    player.set_master_volume(0.5f);
    SIF_CHECK(near(player.gain_of(song), 0.125f));
    SIF_CHECK(near(player.gain_of(blip), 0.4f)); // the Effects bus is untouched

    // A master change no longer throws away each voice's own volume.
    player.set_master_volume(1.f);
    SIF_CHECK(near(player.gain_of(song), 0.25f));

    player.set_volume(song, 1.f);
    SIF_CHECK(near(player.gain_of(song), 0.5f));
    SIF_CHECK(near(player.bus_volume(audio::Bus::Music), 0.5f));
    player.set_bus_volume(audio::Bus::Interface, 7.f);
    SIF_CHECK(player.bus_volume(audio::Bus::Interface) == 1.f);

    player.set_pan(song, -3.f);
    SIF_CHECK(player.pan_of(song) == -1.f);
}

SIF_TEST(audio_fades_follow_update) {
    auto now = std::chrono::steady_clock::time_point{};
    audio::Null_AudioPlayer player(8, [&now] { return now; });
    const auto sound = two_second_sound();

    audio::PlayOptions options;
    options.loop = true;
    options.fade_in_seconds = 1.f;
    const audio::VoiceId v = player.play(sound, options);
    SIF_CHECK(near(player.gain_of(v), 0.f));
    player.update(0.25f);
    SIF_CHECK(near(player.gain_of(v), 0.25f));
    player.update(1.f);
    SIF_CHECK(near(player.gain_of(v), 1.f));

    player.fade_to(v, 0.5f, 0.5f);
    player.update(0.25f);
    SIF_CHECK(near(player.gain_of(v), 0.75f));
    player.set_volume(v, 0.2f); // cancels the fade
    player.update(1.f);
    SIF_CHECK(near(player.gain_of(v), 0.2f));

    player.fade_out(v, 0.5f);
    player.update(0.4f);
    SIF_CHECK(player.is_playing(v));
    player.update(0.2f);
    SIF_CHECK(!player.is_playing(v)); // stopped at the end of the fade
    SIF_CHECK(player.active_voices() == 0);
}

SIF_TEST(audio_pause_and_pitch_change_how_long_a_voice_lasts) {
    using namespace std::chrono_literals;
    auto now = std::chrono::steady_clock::time_point{};
    audio::Null_AudioPlayer player(8, [&now] { return now; });
    const auto sound = two_second_sound();

    const audio::VoiceId paused = player.play(sound);
    now += 1s;
    player.pause(paused);
    SIF_CHECK(player.is_paused(paused) && !player.is_playing(paused));
    SIF_CHECK(player.active_voices() == 0);
    now += 10s; // time spent paused does not count
    player.resume(paused);
    SIF_CHECK(player.is_playing(paused));
    now += 900ms;
    SIF_CHECK(player.is_playing(paused));
    now += 200ms;
    SIF_CHECK(!player.is_playing(paused));

    audio::PlayOptions fast;
    fast.pitch = 2.f; // twice as fast: a 2 s sound lasts 1 s
    const audio::VoiceId v = player.play(sound, fast);
    now += 900ms;
    SIF_CHECK(player.is_playing(v));
    now += 200ms;
    SIF_CHECK(!player.is_playing(v));
}
