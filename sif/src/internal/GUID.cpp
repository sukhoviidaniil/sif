/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-02-08
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/internal/GUID.h"

namespace sif::intrnl {
    GUID::GUID() = default;

    GUID::GUID(const std::uint32_t i) : id(i) {
    }

    GUID::GUID(const std::string &i) {
        std::uint32_t value{};
        const char* begin = i.data();
        const char* end   = i.data() + i.size();

        auto [ptr, ec] = std::from_chars(begin, end, value, 10);

        // the string must be parsed completely and without errors
        if (ec != std::errc{} || ptr != end) {
            throw std::invalid_argument("Invalid GUID string: " + i);
        }

        id = value;
    }

    std::string GUID::string() const  {
        return std::to_string(id);
    }
}
