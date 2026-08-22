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

    inline constexpr VoiceId invalid_voice = 0;

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
         * @param volume Linear volume in [0, 1], multiplied by master_volume().
         * @param loop Whether playback repeats until stopped.
         * @return The new voice id, or invalid_voice if the asset is
         * not ready (or no free voice was available).
         */
        virtual VoiceId play(const asset::AssetHandle<asset::Sound>& sound, float volume = 1.f, bool loop = false) = 0;

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
