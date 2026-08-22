/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-02-08
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_ATTRIBUTE_CHECK_H
#define RENDER_ENGINE_ATTRIBUTE_CHECK_H

#include <stdexcept>
#include <string>

namespace sif::ui {
    inline void missing_attribute(const std::string& from, const std::string& attribute) {
        throw std::runtime_error(from + ": missing required attribute '" + attribute + "'");
    }
    inline void invalid_attribute(const std::string& from, const std::string& attribute, const std::string& at,
                                  const char* what) {
        throw std::runtime_error(from + ": invalid " + attribute + " value '" + at + "': " + what);
    }
} // namespace sif::ui

#endif // RENDER_ENGINE_ATTRIBUTE_CHECK_H