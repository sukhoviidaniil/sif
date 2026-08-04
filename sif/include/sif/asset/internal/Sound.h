/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETS_SOUND_H
#define RENDER_ENGINE_ASSETS_SOUND_H

namespace sif::asset {
    /**
     * @brief A loaded sound sample (AssetType::Sound).
     *
     * Backend-agnostic interface; the actual audio data lives in a
     * backend-specific subclass constructed by that backend's asset
     * loader (e.g. sif::sfml::SFMLSound, built by
     * SFML_Sound_AssetLoader).
     */
    class Sound {
    public:
        Sound(const Sound&) = delete;
        Sound& operator=(const Sound&) = delete;

        Sound();
        virtual ~Sound();
        Sound(Sound&&) noexcept;
        Sound& operator=(Sound&&) noexcept;

        /**
         * @brief Returns the duration of the sound, in seconds.
         */
        [[nodiscard]] virtual float duration_seconds() const = 0;
    };
}

#endif //RENDER_ENGINE_ASSETS_SOUND_H
