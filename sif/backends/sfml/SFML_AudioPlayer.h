/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SFML_SFML_AUDIOPLAYER_H
#define RENDER_ENGINE_SFML_SFML_AUDIOPLAYER_H

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include "SFML/Audio/Sound.hpp"

#include "sif/audio/AudioPlayer.h"

namespace sif::sfml {
    /**
     * @brief SFML implementation of audio::AudioPlayer.
     *
     * Keeps one sf::Sound per active voice. sf::Sound only *references*
     * its sf::SoundBuffer, so the voice also keeps the asset handle's
     * shared_ptr alive for as long as it plays - dropping that would
     * leave SFML reading a freed buffer if the asset were ever
     * reloaded mid-playback.
     *
     * Finished voices are reaped lazily (on the next play/query rather
     * than from a timer), which keeps the class free of any thread of
     * its own.
     */
    class SFML_AudioPlayer final : public audio::AudioPlayer {
    public:
        explicit SFML_AudioPlayer(std::size_t max_voices = 32);
        ~SFML_AudioPlayer() override;

        using audio::AudioPlayer::play; // keep play(sound, volume, loop) visible

        audio::VoiceId play(const asset::AssetHandle<asset::Sound>& sound,
                            const audio::PlayOptions& options) override;

        void stop(audio::VoiceId voice) override;
        void stop_all() override;

        [[nodiscard]] bool is_playing(audio::VoiceId voice) const override;
        [[nodiscard]] std::size_t active_voices() const override;

        void set_master_volume(float volume) override;
        [[nodiscard]] float master_volume() const override;

        void set_volume(audio::VoiceId voice, float volume) override;
        void set_pitch(audio::VoiceId voice, float pitch) override;
        void set_pan(audio::VoiceId voice, float pan) override;
        void pause(audio::VoiceId voice) override;
        void resume(audio::VoiceId voice) override;
        [[nodiscard]] bool is_paused(audio::VoiceId voice) const override;
        void fade_to(audio::VoiceId voice, float target, float seconds) override;
        void fade_out(audio::VoiceId voice, float seconds) override;

        void set_bus_volume(audio::Bus bus, float volume) override;
        [[nodiscard]] float bus_volume(audio::Bus bus) const override;

        void update(float dt) override;

    private:
        struct Voice {
            audio::VoiceId id = audio::invalid_voice;
            std::unique_ptr<sf::Sound> sound;
            std::shared_ptr<void> keep_alive; ///< Owns the buffer while it plays
            float volume = 1.f;               ///< The voice's own volume, before bus and master
            audio::Bus bus = audio::Bus::Effects;
            std::optional<audio::Fade> fade;
        };

        void reap_finished();

        /// @brief The voice with this id, or nullptr.
        Voice* find(audio::VoiceId voice);
        [[nodiscard]] const Voice* find(audio::VoiceId voice) const;

        /**
         * @brief Pushes voice x bus x master into sf::Sound.
         *
         * The one place a voice's loudness is computed - it used to be
         * set to master x 100 on every master change, which silently
         * threw away each voice's own volume.
         */
        void apply_gain(Voice& v) const;

        std::vector<Voice> voices_;
        std::size_t max_voices_;
        audio::VoiceId next_id_ = 1;
        float master_volume_ = 1.f;
        std::array<float, audio::bus_count> bus_volumes_{1.f, 1.f, 1.f};
    };
}

#endif //RENDER_ENGINE_SFML_SFML_AUDIOPLAYER_H
