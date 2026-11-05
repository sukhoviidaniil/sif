/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2026-11-05
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#include "sif/render/Layer.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace sif::rnd {
int layer_from_string(const std::string_view str) {
    if (str == "Background")
        return layer::Background;
    if (str == "Content")
        return layer::Content;
    if (str == "Popup")
        return layer::Popup;
    if (str == "Modal")
        return layer::Modal;
    if (str == "Tooltip")
        return layer::Tooltip;

    // Same "the whole string must be consumed" rule as intrnl::Size,
    // so "10px" or "Modal2" is an error instead of silently becoming 10
    // or 0.
    const std::string tmp(str);
    char* end = nullptr;
    errno = 0;
    const long v = std::strtol(tmp.c_str(), &end, 10);
    if (tmp.empty() || end != tmp.c_str() + tmp.size()) {
        throw std::invalid_argument("Layer: expected Background|Content|Popup|Modal|Tooltip or an integer");
    }
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) {
        throw std::invalid_argument("Layer: value out of range");
    }
    return static_cast<int>(v);
}
} // namespace sif::rnd
