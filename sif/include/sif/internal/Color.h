/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-22
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_COLOR_H
#define RENDER_ENGINE_COLOR_H

#include <cstdint>
#include <string>
#include <string_view>

namespace sif::intrnl {
    class Color {
    public:
        Color();

        Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255);

        explicit Color(std::string_view s, std::string separator = "|");

        std::uint8_t r = 255, g = 255, b = 255;
        std::uint8_t a = 255;
    };
} // namespace sif::intrnl

#endif // RENDER_ENGINE_COLOR_H