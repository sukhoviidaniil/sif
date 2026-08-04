/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-02-08
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/asset/internal/Font.h"
namespace sif::asset {
    Font::Font() = default;

    Font::~Font() = default;

    Font::Font(Font &&) noexcept = default;

    Font & Font::operator=(Font &&) noexcept = default;

}
