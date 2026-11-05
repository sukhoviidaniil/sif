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

#include <cmath>

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
                                          const audio::PlayOptions &options) {
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
        voice.volume = std::clamp(options.volume, 0.f, 1.f);
        voice.bus = options.bus;
        if (options.fade_in_seconds > 0.f) {
            voice.fade = audio::Fade{0.f, voice.volume, options.fade_in_seconds, 0.f, false};
        }
        voice.sound->setLoop(options.loop);
        voices_.push_back(std::move(voice));

        Voice& added = voices_.back();
        apply_gain(added);
        const audio::VoiceId id = added.id;
        set_pitch(id, options.pitch);
        set_pan(id, options.pan);
        added.sound->play();
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
            apply_gain(v);
        }
    }

    float SFML_AudioPlayer::master_volume() const {
        return master_volume_;
    }

    SFML_AudioPlayer::Voice* SFML_AudioPlayer::find(const audio::VoiceId voice) {
        for (Voice& v : voices_) {
            if (v.id == voice && v.sound != nullptr) {
                return &v;
            }
        }
        return nullptr;
    }

    const SFML_AudioPlayer::Voice* SFML_AudioPlayer::find(const audio::VoiceId voice) const {
        for (const Voice& v : voices_) {
            if (v.id == voice && v.sound != nullptr) {
                return &v;
            }
        }
        return nullptr;
    }

    void SFML_AudioPlayer::apply_gain(Voice &v) const {
        if (v.sound == nullptr) {
            return;
        }
        const float voice = v.fade.has_value() ? v.fade->value() : v.volume;
        const float bus = bus_volumes_[static_cast<std::size_t>(v.bus)];
        v.sound->setVolume(std::clamp(voice, 0.f, 1.f) * bus * master_volume_ * 100.f);
    }

    void SFML_AudioPlayer::set_volume(const audio::VoiceId voice, const float volume) {
        if (Voice* v = find(voice)) {
            v->volume = std::clamp(volume, 0.f, 1.f);
            v->fade.reset();
            apply_gain(*v);
        }
    }

    void SFML_AudioPlayer::set_pitch(const audio::VoiceId voice, const float pitch) {
        if (Voice* v = find(voice)) {
            v->sound->setPitch(std::max(0.01f, pitch));
        }
    }

    void SFML_AudioPlayer::set_pan(const audio::VoiceId voice, const float pan) {
        if (Voice* v = find(voice)) {
            // OpenAL has no pan control, only 3D position: put the source
            // on a unit circle around the listener, left to right, so the
            // distance (and thus loudness) never changes. OpenAL
            // spatialises mono buffers only - a stereo sound keeps its
            // own channels whatever this is set to.
            const float p = std::clamp(pan, -1.f, 1.f);
            v->sound->setRelativeToListener(true);
            v->sound->setMinDistance(1.f);
            v->sound->setPosition(p, 0.f, -std::sqrt(std::max(0.f, 1.f - p * p)));
        }
    }

    void SFML_AudioPlayer::pause(const audio::VoiceId voice) {
        if (Voice* v = find(voice); v != nullptr && v->sound->getStatus() == sf::Sound::Playing) {
            v->sound->pause();
        }
    }

    void SFML_AudioPlayer::resume(const audio::VoiceId voice) {
        if (Voice* v = find(voice); v != nullptr && v->sound->getStatus() == sf::Sound::Paused) {
            v->sound->play();
        }
    }

    bool SFML_AudioPlayer::is_paused(const audio::VoiceId voice) const {
        const Voice* v = find(voice);
        return v != nullptr && v->sound->getStatus() == sf::Sound::Paused;
    }

    void SFML_AudioPlayer::fade_to(const audio::VoiceId voice, const float target, const float seconds) {
        if (Voice* v = find(voice)) {
            const float from = v->fade.has_value() ? v->fade->value() : v->volume;
            v->fade = audio::Fade{from, std::clamp(target, 0.f, 1.f), std::max(0.f, seconds), 0.f, false};
            apply_gain(*v);
        }
    }

    void SFML_AudioPlayer::fade_out(const audio::VoiceId voice, const float seconds) {
        fade_to(voice, 0.f, seconds);
        if (Voice* v = find(voice); v != nullptr && v->fade.has_value()) {
            v->fade->stop_at_end = true;
        }
    }

    void SFML_AudioPlayer::set_bus_volume(const audio::Bus bus, const float volume) {
        bus_volumes_[static_cast<std::size_t>(bus)] = std::clamp(volume, 0.f, 1.f);
        for (Voice& v : voices_) {
            apply_gain(v);
        }
    }

    float SFML_AudioPlayer::bus_volume(const audio::Bus bus) const {
        return bus_volumes_[static_cast<std::size_t>(bus)];
    }

    void SFML_AudioPlayer::update(const float dt) {
        for (Voice& v : voices_) {
            if (!v.fade.has_value() || v.sound == nullptr) {
                continue;
            }
            v.fade->elapsed += std::max(0.f, dt);
            apply_gain(v);
            if (v.fade->finished()) {
                v.volume = v.fade->to;
                const bool stop = v.fade->stop_at_end;
                v.fade.reset();
                if (stop) {
                    v.sound->stop();
                }
            }
        }
        reap_finished();
    }
}
