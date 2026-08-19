/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "SFML_AudioPlayer.h"

#include <algorithm>

#include "SFMLAssets.h"

#include "sif/diagnostics/Logger.h"

namespace sif::sfml {
    SFML_AudioPlayer::SFML_AudioPlayer(const std::size_t max_voices)
        : max_voices_(std::max<std::size_t>(1, max_voices)) {
    }

    SFML_AudioPlayer::~SFML_AudioPlayer() {
        stop_all();
    }

    void SFML_AudioPlayer::reap_finished() {
        std::erase_if(voices_, [](const Voice& v) {
            return v.sound == nullptr || v.sound->getStatus() == sf::Sound::Stopped;
        });
    }

    audio::VoiceId SFML_AudioPlayer::play(const asset::AssetHandle<asset::Sound> &sound,
                                          const float volume,
                                          const bool loop) {
        reap_finished();

        // Not being loaded yet is a normal, expected state (loads run
        // in the background), so this reports "no voice" instead of
        // throwing at the caller.
        const auto record = sound.record().lock();
        if (record == nullptr || !sound.ready()) {
            LOG("SFML_AudioPlayer::play - sound is not ready, skipping");
            return audio::invalid_voice;
        }

        if (voices_.size() >= max_voices_) {
            LOG("SFML_AudioPlayer::play - voice limit reached, skipping");
            return audio::invalid_voice;
        }

        const auto* sfml_sound = static_cast<const SFMLSoundAsset*>(sound.get());
        if (sfml_sound == nullptr) {
            return audio::invalid_voice;
        }

        Voice voice;
        voice.id = next_id_++;
        voice.sound = std::make_unique<sf::Sound>(sfml_sound->buffer());
        voice.keep_alive = record->get_data();
        voice.sound->setLoop(loop);
        voice.sound->setVolume(std::clamp(volume, 0.f, 1.f) * master_volume_ * 100.f);
        voice.sound->play();

        const audio::VoiceId id = voice.id;
        voices_.push_back(std::move(voice));
        return id;
    }

    void SFML_AudioPlayer::stop(const audio::VoiceId voice) {
        for (Voice& v : voices_) {
            if (v.id == voice && v.sound != nullptr) {
                v.sound->stop();
            }
        }
        reap_finished();
    }

    void SFML_AudioPlayer::stop_all() {
        for (Voice& v : voices_) {
            if (v.sound != nullptr) {
                v.sound->stop();
            }
        }
        voices_.clear();
    }

    bool SFML_AudioPlayer::is_playing(const audio::VoiceId voice) const {
        for (const Voice& v : voices_) {
            if (v.id == voice && v.sound != nullptr) {
                return v.sound->getStatus() == sf::Sound::Playing;
            }
        }
        return false;
    }

    std::size_t SFML_AudioPlayer::active_voices() const {
        return static_cast<std::size_t>(std::count_if(voices_.begin(), voices_.end(), [](const Voice& v) {
            return v.sound != nullptr && v.sound->getStatus() == sf::Sound::Playing;
        }));
    }

    void SFML_AudioPlayer::set_master_volume(const float volume) {
        master_volume_ = std::clamp(volume, 0.f, 1.f);
        for (Voice& v : voices_) {
            if (v.sound != nullptr) {
                v.sound->setVolume(master_volume_ * 100.f);
            }
        }
    }

    float SFML_AudioPlayer::master_volume() const {
        return master_volume_;
    }
}
