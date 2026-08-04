/***************************************************************
 * Project:       Render_Engine (SFML backend)
 * File:          SFML_AudioPlayer.h
 *
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

#include <memory>
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

        audio::VoiceId play(const asset::AssetHandle<asset::Sound>& sound,
                            float volume = 1.f,
                            bool loop = false) override;

        void stop(audio::VoiceId voice) override;
        void stop_all() override;

        [[nodiscard]] bool is_playing(audio::VoiceId voice) const override;
        [[nodiscard]] std::size_t active_voices() const override;

        void set_master_volume(float volume) override;
        [[nodiscard]] float master_volume() const override;

    private:
        struct Voice {
            audio::VoiceId id = audio::invalid_voice;
            std::unique_ptr<sf::Sound> sound;
            std::shared_ptr<void> keep_alive; ///< Owns the buffer while it plays
        };

        void reap_finished();

        std::vector<Voice> voices_;
        std::size_t max_voices_;
        audio::VoiceId next_id_ = 1;
        float master_volume_ = 1.f;
    };
}

#endif //RENDER_ENGINE_SFML_SFML_AUDIOPLAYER_H
