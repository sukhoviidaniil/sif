/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-14
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_ASSETS_FONT_H
#define RENDER_ENGINE_ASSETS_FONT_H

#include <string>

#include "sif/math/Vector2.h"

namespace sif::asset {
class Font {
public:
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    Font();
    virtual ~Font();
    Font(Font&&) noexcept;
    Font& operator=(Font&&) noexcept;

    /**
     * @brief Size of str drawn at the given character size.
     *
     * str is UTF-8 - the encoding *.ui.xml scenes are read in and the
     * one ui::Text stores - so implementations must measure characters,
     * not bytes, and decode it the same way their renderer does.
     */
    [[nodiscard]] virtual math::Vector2 measure_text(const std::string& str, unsigned int size) const = 0;
};
} // namespace sif::asset

#endif // RENDER_ENGINE_ASSETS_FONT_H