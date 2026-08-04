/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-15
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_RECORDID_H
#define RENDER_ENGINE_RECORDID_H
#include <cstdint>
#include <string>

namespace sif::intrnl {
    struct RecordID {
        std::uint32_t id = 0;

        RecordID() = default;
        explicit RecordID(const std::uint32_t i) : id(i){}

        // equality
        friend constexpr bool operator==(const RecordID& a, const RecordID& b) noexcept {
            return a.id == b.id;
        }

        // ordering
        friend constexpr bool operator<(const RecordID& a, const RecordID& b) noexcept {
            return a.id < b.id;
        }

        friend constexpr bool operator>(const RecordID& a, const RecordID& b) noexcept {
            return a.id > b.id;
        }

        [[nodiscard]] std::string string() const {
            return std::to_string(id);
        }
    };

    struct RecordIDHash {
        size_t operator()(const RecordID& g) const noexcept {
            return std::hash<std::uint32_t>{}(g.id);
        }
    };
}

#endif //RENDER_ENGINE_RECORDID_H