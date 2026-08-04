/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-22
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#include "sif/internal/Color.h"

#include <charconv>
#include <stdexcept>

namespace sif::intrnl {
    Color::Color() = default;

    Color::Color(const std::uint8_t red, const std::uint8_t green, const std::uint8_t blue, const std::uint8_t alpha)
        : r(red), g(green), b(blue), a(alpha) {
    }

    Color::Color(std::string_view s, std::string separator)  {

        std::uint32_t values[4] = {};
        std::size_t count = 0;

        std::size_t pos = 0;
        while (pos < s.size() && count < 4) {
            std::size_t next = s.find(separator, pos);
            if (next == std::string_view::npos)
                next = s.size();

            std::uint32_t v{};
            auto part = s.substr(pos, next - pos);

            auto [ptr, ec] =
                std::from_chars(part.data(), part.data() + part.size(), v, 10);

            if (ec != std::errc{} || ptr != part.data() + part.size() || v > 255)
                throw std::invalid_argument("Invalid color component");

            values[count++] = v;
            pos = next + 1;
        }

        if (count != 3 && count != 4) {
            throw std::invalid_argument(
                "Color must be r"+separator+"g"+separator+"b or r" + separator+ "g"+separator+"b"+separator+"a"
                );
        }

        r = static_cast<std::uint8_t>(values[0]);
        g = static_cast<std::uint8_t>(values[1]);
        b = static_cast<std::uint8_t>(values[2]);
        a = (count == 4)
              ? static_cast<std::uint8_t>(values[3])
              : static_cast<std::uint8_t>(255);
    }
}
