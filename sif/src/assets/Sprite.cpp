/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/asset/internal/Sprite.h"

namespace sif::asset {
    Sprite::Sprite() = default;

    Sprite::~Sprite() = default;

    Sprite::Sprite(Sprite&&) noexcept = default;

    Sprite& Sprite::operator=(Sprite&&) noexcept = default;
}
