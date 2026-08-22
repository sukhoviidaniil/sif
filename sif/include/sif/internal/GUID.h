/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-13
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_GUID_H
#define RENDER_ENGINE_GUID_H

#include <charconv>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "Random.h"

namespace sif::intrnl {
    struct GUID {
        std::uint32_t id = 0;

        GUID();
        explicit GUID(std::uint32_t i);
        explicit GUID(const std::string& i);

        [[nodiscard]] std::string string() const;

        // equality
        friend constexpr bool operator==(const GUID& a, const GUID& b) noexcept { return a.id == b.id; }

        // ordering
        friend constexpr bool operator<(const GUID& a, const GUID& b) noexcept { return a.id < b.id; }

        friend constexpr bool operator>(const GUID& a, const GUID& b) noexcept { return a.id > b.id; }
    };

    struct GUIDHash {
        size_t operator()(const GUID& g) const noexcept { return std::hash<std::uint32_t>{}(g.id); }
    };

    inline GUID generate_guid(const std::unordered_set<GUID, GUIDHash>& existing) {
        GUID g;
        do {
            // Drawn as uint32_t, not as int: casting the uint32_t
            // maximum to int yielded -1, so the range handed to the
            // distribution was [1, -1] - an empty range, which is
            // undefined behaviour rather than an error.
            g = GUID{Random::instance().next_int<std::uint32_t>(1, std::numeric_limits<std::uint32_t>::max())};
        } while (existing.contains(g));

        return g;
    }
} // namespace sif::intrnl

#endif // RENDER_ENGINE_GUID_H