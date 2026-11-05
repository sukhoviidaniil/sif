/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-08-03
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_AUDIOPLAYER_H
#define RENDER_ENGINE_AUDIOPLAYER_H

#include <cstddef>
#include <cstdint>

#include "sif/asset/AssetHandle.h"
#include "sif/asset/internal/Sound.h"

namespace sif::audio {
/**
 * @brief Identifies one playing instance of a sound ("voice").
 *
 * Returned by AudioPlayer::play so the caller can stop or query
 * that one playback without affecting other instances of the same
 * asset. 0 is never a valid voice and means "could not play".
 */
using VoiceId = std::uint32_t;

/**
 * this is a "zero" value, reserved as a "no sound was played at all" signal
 */
inline constexpr VoiceId invalid_voice = 0;

/**
 * @brief Mixer bus a voice plays on.
 *
 * Every voice's loudness is its own volume x its bus's volume x the
 * master volume, so an options menu can have separate sliders for music
 * and effects without touching a single play() call.
 */
enum class Bus : std::uint8_t {
    Effects,  ///< Default: gameplay sounds
    Music,    ///< Background music
    Interface ///< Menu clicks and other UI feedback
};

/// @brief Number of Bus values (for per-bus tables).
inline constexpr std::size_t bus_count = 3;

/**
 * @brief Everything play() can be told about a new voice.
 */
struct PlayOptions {
    float volume = 1.f;          ///< Linear voice volume in [0, 1]
    bool loop = false;           ///< Repeat until stopped
    Bus bus = Bus::Effects;      ///< Mixer bus
    float pitch = 1.f;           ///< Playback speed/pitch factor (1 = as recorded)
    float pan = 0.f;             ///< -1 = left, 0 = centre, 1 = right (mono sounds; see the backend)
    float fade_in_seconds = 0.f; ///< Ramp from silence to volume over this long
};

/**
 * @brief A linear volume ramp, advanced by AudioPlayer::update.
 *
 * Plain data plus the arithmetic, shared by every backend so a fade
 * sounds the same whichever one plays it.
 */
struct Fade {
    float from = 0.f;
    float to = 0.f;
    float duration = 0.f;
    float elapsed = 0.f;
    bool stop_at_end = false; ///< Stop the voice when the ramp completes (fade_out)

    /// @brief Volume at the current point of the ramp.
    [[nodiscard]] float value() const {
        if (duration <= 0.f || elapsed >= duration) {
            return to;
        }
        return from + (to - from) * (elapsed / duration);
    }

    [[nodiscard]] bool finished() const { return elapsed >= duration; }
};

/**
 * @brief Backend-agnostic audio output.
 *
 * The mirror image of rnd::Renderer: the engine describes *what*
 * should be heard (an asset::Sound handle plus volume/looping) and
 * a backend implementation - e.g. sif::sfml::SFML_AudioPlayer -
 * decides how. Keeping this interface in the engine is what allows
 * sound-driven logic (a menu click, an animation that ends with a
 * thud) to live next to the rest of the game logic while the
 * engine itself still links without any audio library.
 *
 * Playing a sound that has not finished loading is not an error:
 * implementations return invalid_voice and carry on, so callers
 * never have to poll AssetState before making noise.
 */
class AudioPlayer {
public:
    AudioPlayer() = default;
    virtual ~AudioPlayer();

    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    /**
     * @brief Starts playing a sound.
     *
     * @param sound Handle to the sound asset.
     * @param options Volume, looping, bus, pitch, pan and fade-in.
     * @return The new voice id, or invalid_voice if the asset is
     * not ready (or no free voice was available).
     */
    virtual VoiceId play(const asset::AssetHandle<asset::Sound>& sound, const PlayOptions& options) = 0;

    /**
     * @brief Starts playing a sound on the Effects bus (the original
     * signature, kept for every existing caller).
     *
     * @param volume Linear volume in [0, 1], scaled by bus and master volume.
     * @param loop Whether playback repeats until stopped.
     */
    VoiceId play(const asset::AssetHandle<asset::Sound>& sound, float volume = 1.f, bool loop = false);

    // ===== One voice =====

    /// @brief Sets a voice's own volume ([0, 1]); cancels a fade in progress.
    virtual void set_volume(VoiceId voice, float volume) = 0;

    /// @brief Sets a voice's pitch/speed factor (> 0; 1 = as recorded).
    virtual void set_pitch(VoiceId voice, float pitch) = 0;

    /// @brief Sets a voice's stereo position, -1 (left) to 1 (right).
    virtual void set_pan(VoiceId voice, float pan) = 0;

    /// @brief Pauses a voice, keeping its position; is_playing() is false while paused.
    virtual void pause(VoiceId voice) = 0;

    /// @brief Continues a paused voice from where it was.
    virtual void resume(VoiceId voice) = 0;

    [[nodiscard]] virtual bool is_paused(VoiceId voice) const = 0;

    /// @brief Ramps a voice's volume to target over seconds (see update()).
    virtual void fade_to(VoiceId voice, float target, float seconds) = 0;

    /// @brief Ramps a voice to silence over seconds, then stops it.
    virtual void fade_out(VoiceId voice, float seconds) = 0;

    // ===== Mixer =====

    /// @brief Scales every voice on a bus; clamped to [0, 1].
    virtual void set_bus_volume(Bus bus, float volume) = 0;

    [[nodiscard]] virtual float bus_volume(Bus bus) const = 0;

    /**
     * @brief Advances time-based state (fades) by dt seconds.
     *
     * Called once per frame by the frame loop, like UIElement::update;
     * nothing else in a player depends on wall-clock time.
     */
    virtual void update(float dt) = 0;

    /// @brief Stops one voice; a no-op if it already finished.
    virtual void stop(VoiceId voice) = 0;

    /// @brief Stops every currently playing voice.
    virtual void stop_all() = 0;

    /// @brief True while the given voice is still producing sound.
    [[nodiscard]] virtual bool is_playing(VoiceId voice) const = 0;

    /// @brief Number of voices currently producing sound.
    [[nodiscard]] virtual std::size_t active_voices() const = 0;

    /// @brief Scales every voice; clamped to [0, 1].
    virtual void set_master_volume(float volume) = 0;

    [[nodiscard]] virtual float master_volume() const = 0;
};
} // namespace sif::audio

#endif // RENDER_ENGINE_AUDIOPLAYER_H
