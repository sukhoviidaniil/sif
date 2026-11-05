/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2026-11-05
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#include "sif/audio/Null_AudioPlayer.h"

#include <algorithm>

namespace sif::audio {
Null_AudioPlayer::Null_AudioPlayer(const std::size_t max_voices, Clock clock)
    : max_voices_(std::max<std::size_t>(1, max_voices)), clock_(std::move(clock)) {}

Null_AudioPlayer::~Null_AudioPlayer() = default;

double Null_AudioPlayer::played(const Voice& v) const {
    if (v.paused) {
        return v.played_before;
    }
    return v.played_before + std::chrono::duration<double>(clock_() - v.resumed).count() * v.pitch;
}

bool Null_AudioPlayer::alive(const Voice& v) const { return v.loop || played(v) < v.duration_seconds; }

Null_AudioPlayer::Voice* Null_AudioPlayer::find(const VoiceId voice) {
    for (Voice& v : voices_) {
        if (v.id == voice && alive(v)) {
            return &v;
        }
    }
    return nullptr;
}

const Null_AudioPlayer::Voice* Null_AudioPlayer::find(const VoiceId voice) const {
    for (const Voice& v : voices_) {
        if (v.id == voice && alive(v)) {
            return &v;
        }
    }
    return nullptr;
}

VoiceId Null_AudioPlayer::play(const asset::AssetHandle<asset::Sound>& sound, const PlayOptions& options) {
    std::erase_if(voices_, [this](const Voice& v) { return !alive(v); });

    // Same contract as a real player: a sound that is not loaded yet,
    // or no free voice, means "no voice" rather than an error.
    const asset::Sound* s = sound.get();
    if (s == nullptr || voices_.size() >= max_voices_) {
        return invalid_voice;
    }

    Voice v;
    v.id = next_id_++;
    v.duration_seconds = s->duration_seconds();
    v.loop = options.loop;
    v.resumed = clock_();
    v.pitch = std::max(0.01f, options.pitch);
    v.volume = std::clamp(options.volume, 0.f, 1.f);
    v.bus = options.bus;
    v.pan = std::clamp(options.pan, -1.f, 1.f);
    if (options.fade_in_seconds > 0.f) {
        v.fade = Fade{0.f, v.volume, options.fade_in_seconds, 0.f, false};
    }
    voices_.push_back(v);
    return v.id;
}

void Null_AudioPlayer::stop(const VoiceId voice) {
    std::erase_if(voices_, [voice](const Voice& v) { return v.id == voice; });
}

void Null_AudioPlayer::stop_all() { voices_.clear(); }

bool Null_AudioPlayer::is_playing(const VoiceId voice) const {
    const Voice* v = find(voice);
    return v != nullptr && !v->paused;
}

std::size_t Null_AudioPlayer::active_voices() const {
    return static_cast<std::size_t>(
        std::count_if(voices_.begin(), voices_.end(), [this](const Voice& v) { return alive(v) && !v.paused; }));
}

void Null_AudioPlayer::set_master_volume(const float volume) { master_volume_ = std::clamp(volume, 0.f, 1.f); }

float Null_AudioPlayer::master_volume() const { return master_volume_; }

void Null_AudioPlayer::set_volume(const VoiceId voice, const float volume) {
    if (Voice* v = find(voice)) {
        v->volume = std::clamp(volume, 0.f, 1.f);
        v->fade.reset();
    }
}

void Null_AudioPlayer::set_pitch(const VoiceId voice, const float pitch) {
    if (Voice* v = find(voice)) {
        // Fold the time played at the old pitch in before changing it.
        v->played_before = played(*v);
        v->resumed = clock_();
        v->pitch = std::max(0.01f, pitch);
    }
}

void Null_AudioPlayer::set_pan(const VoiceId voice, const float pan) {
    if (Voice* v = find(voice)) {
        v->pan = std::clamp(pan, -1.f, 1.f);
    }
}

void Null_AudioPlayer::pause(const VoiceId voice) {
    if (Voice* v = find(voice); v != nullptr && !v->paused) {
        v->played_before = played(*v);
        v->paused = true;
    }
}

void Null_AudioPlayer::resume(const VoiceId voice) {
    if (Voice* v = find(voice); v != nullptr && v->paused) {
        v->resumed = clock_();
        v->paused = false;
    }
}

bool Null_AudioPlayer::is_paused(const VoiceId voice) const {
    const Voice* v = find(voice);
    return v != nullptr && v->paused;
}

void Null_AudioPlayer::fade_to(const VoiceId voice, const float target, const float seconds) {
    if (Voice* v = find(voice)) {
        const float from = v->fade.has_value() ? v->fade->value() : v->volume;
        v->fade = Fade{from, std::clamp(target, 0.f, 1.f), std::max(0.f, seconds), 0.f, false};
    }
}

void Null_AudioPlayer::fade_out(const VoiceId voice, const float seconds) {
    fade_to(voice, 0.f, seconds);
    if (Voice* v = find(voice); v != nullptr && v->fade.has_value()) {
        v->fade->stop_at_end = true;
    }
}

void Null_AudioPlayer::set_bus_volume(const Bus bus, const float volume) {
    bus_volumes_[static_cast<std::size_t>(bus)] = std::clamp(volume, 0.f, 1.f);
}

float Null_AudioPlayer::bus_volume(const Bus bus) const { return bus_volumes_[static_cast<std::size_t>(bus)]; }

void Null_AudioPlayer::update(const float dt) {
    std::vector<VoiceId> to_stop;
    for (Voice& v : voices_) {
        if (!v.fade.has_value()) {
            continue;
        }
        v.fade->elapsed += std::max(0.f, dt);
        if (v.fade->finished()) {
            v.volume = v.fade->to;
            if (v.fade->stop_at_end) {
                to_stop.push_back(v.id);
            }
            v.fade.reset();
        }
    }
    for (const VoiceId id : to_stop) {
        stop(id);
    }
}

float Null_AudioPlayer::gain_of(const VoiceId voice) const {
    const Voice* v = find(voice);
    if (v == nullptr) {
        return 0.f;
    }
    const float own = v->fade.has_value() ? v->fade->value() : v->volume;
    return own * bus_volumes_[static_cast<std::size_t>(v->bus)] * master_volume_;
}

float Null_AudioPlayer::pan_of(const VoiceId voice) const {
    const Voice* v = find(voice);
    return v == nullptr ? 0.f : v->pan;
}
} // namespace sif::audio
