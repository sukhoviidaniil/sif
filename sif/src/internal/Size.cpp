/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-02-08
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/internal/Size.h"
#include <stdexcept>

namespace sif::intrnl {
    Size Size::auto_() {
        // 0, to match what Size("AUTO|...") parses to: an Auto size
        // ignores its value, and having the two spellings disagree was
        // a trap for anyone comparing Size objects.
        return {Type::Auto, 0.f};
    }

    Size Size::px(float v) {
        return {Type::Pixel, v};
    }

    Size Size::percent(float v) {
        return {Type::Percent, v};
    }

    Size::Size(const Type t, const float v) : type(t), value(v) {}

    Size::Size(std::string_view str) {
        const auto sep = str.find('|');
        if (sep == std::string_view::npos) {
            throw std::invalid_argument("Size: invalid format");
        }

        const auto type_sv = str.substr(0, sep);
        const auto value_sv = str.substr(sep + 1);

        if (type_sv == "AUTO") {
            type = Type::Auto;
            value = 0.f;
            return;
        }

        const std::string tmp(value_sv);

        char* end = nullptr;
        const float v = std::strtof(tmp.c_str(), &end);
        if (end != tmp.c_str() + tmp.size()) {
            throw std::invalid_argument("Size: invalid value");
        }

        if (type_sv == "PX") {
            type = Type::Pixel;
            value = v;
        } else if (type_sv == "PERCENT") {
            type = Type::Percent;
            value = v;
        } else {
            throw std::invalid_argument("Size: unknown type");
        }
    }
} // namespace sif::intrnl
