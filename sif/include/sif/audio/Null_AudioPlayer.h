/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2026-11-05
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_NULL_AUDIOPLAYER_H
#define RENDER_ENGINE_NULL_AUDIOPLAYER_H

#include <array>
#include <chrono>
#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

#include "sif/audio/AudioPlayer.h"

namespace sif::audio {
/**
 * @brief An AudioPlayer that plays nothing but keeps honest books.
 *
 * The audio leg of the headless backend (Null Object): play() hands
 * out real VoiceIds, a voice reports is_playing() for exactly as long
 * as its sound lasts (asset::Sound::duration_seconds, scaled by pitch,
 * not counting time spent paused), looping voices until they are
 * stopped, and volumes, buses and fades are tracked exactly as a real
 * backend mixes them (see gain_of) - so code that reacts to "is that
 * sound still playing" or "how loud is it" behaves the same with or
 * without a sound card. The clock is injectable, so tests can step time
 * instead of sleeping.
 */
class Null_AudioPlayer final : public AudioPlayer {
public:
    using Clock = std::function<std::chrono::steady_clock::time_point()>;

    explicit Null_AudioPlayer(std::size_t max_voices = 32, Clock clock = std::chrono::steady_clock::now);
    ~Null_AudioPlayer() override;

    using AudioPlayer::play; // keep play(sound, volume, loop) visible

    VoiceId play(const asset::AssetHandle<asset::Sound>& sound, const PlayOptions& options) override;
    void stop(VoiceId voice) override;
    void stop_all() override;
    [[nodiscard]] bool is_playing(VoiceId voice) const override;
    [[nodiscard]] std::size_t active_voices() const override;
    void set_master_volume(float volume) override;
    [[nodiscard]] float master_volume() const override;

    void set_volume(VoiceId voice, float volume) override;
    void set_pitch(VoiceId voice, float pitch) override;
    void set_pan(VoiceId voice, float pan) override;
    void pause(VoiceId voice) override;
    void resume(VoiceId voice) override;
    [[nodiscard]] bool is_paused(VoiceId voice) const override;
    void fade_to(VoiceId voice, float target, float seconds) override;
    void fade_out(VoiceId voice, float seconds) override;

    void set_bus_volume(Bus bus, float volume) override;
    [[nodiscard]] float bus_volume(Bus bus) const override;

    void update(float dt) override;

    /// @brief Loudness the voice would be mixed at (voice x bus x master), 0 if it is not alive.
    [[nodiscard]] float gain_of(VoiceId voice) const;

    /// @brief Pan of a live voice (0 if unknown).
    [[nodiscard]] float pan_of(VoiceId voice) const;

private:
    struct Voice {
        VoiceId id = invalid_voice;
        double duration_seconds = 0.0;
        bool loop = false;

        /// Seconds of the sound already played up to `resumed`, pitch included.
        double played_before = 0.0;
        std::chrono::steady_clock::time_point resumed;
        bool paused = false;
        float pitch = 1.f;

        float volume = 1.f;
        Bus bus = Bus::Effects;
        float pan = 0.f;
        std::optional<Fade> fade;
    };

    [[nodiscard]] double played(const Voice& v) const;
    [[nodiscard]] bool alive(const Voice& v) const;
    Voice* find(VoiceId voice);
    [[nodiscard]] const Voice* find(VoiceId voice) const;

    std::size_t max_voices_;
    Clock clock_;
    std::vector<Voice> voices_;
    VoiceId next_id_ = 1;
    float master_volume_ = 1.f;
    std::array<float, bus_count> bus_volumes_{1.f, 1.f, 1.f};
};
} // namespace sif::audio

#endif // RENDER_ENGINE_NULL_AUDIOPLAYER_H
