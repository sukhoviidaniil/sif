/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/asset/internal/Sound.h"

namespace sif::asset {
    Sound::Sound() = default;

    Sound::~Sound() = default;

    Sound::Sound(Sound&&) noexcept = default;

    Sound& Sound::operator=(Sound&&) noexcept = default;
}
