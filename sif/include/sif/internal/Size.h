/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-19
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_SIZE_H
#define RENDER_ENGINE_SIZE_H

#include <string_view>

namespace sif::intrnl {
/**
 * @brief Size specification used by the layout engine.
 *
 * Represents how a dimension (width or height) should be resolved:
 * automatically, in absolute pixels, relative percentages, or
 * logical layout units.
 */
class Size {
public:
    /**
     * @brief Size interpretation mode.
     */
    enum class Type {
        Auto,    ///< Size is determined automatically by layout rules
        Pixel,   ///< Absolute size in pixels
        Percent, ///< Size relative to the parent, in percent: 50 = half (0–100)
    };

    Type type = Type::Auto; ///< Size mode
    float value = 0.f;      ///< Associated value (meaning depends on type)

    /// @brief Automatic size.
    static Size auto_();

    /// @brief Absolute size in pixels.
    static Size px(float v);

    /// @brief Relative size in percent of parent.
    static Size percent(float v);

    Size(Type t, float v);

    explicit Size(std::string_view str);

    /**
     * @brief Resolves this size against the space the parent offers.
     *
     * The single definition of what a Size means, shared by every
     * element (UIElement::resolve_size, Rectangle, Button, Overlay):
     * Pixel is the value itself, Percent is that percentage of
     * available (PERCENT|50 = half - it used to be multiplied
     * directly, so PERCENT|50 meant fifty times the parent), and Auto
     * takes all of available, which is how leaf elements fill their
     * slot. Containers that size to their content check for Auto
     * before calling this.
     *
     * @param available Space offered by the parent along this axis.
     * @return Resolved length (not clamped to min/max).
     */
    [[nodiscard]] float resolve(float available) const;
};
} // namespace sif::intrnl

#endif // RENDER_ENGINE_SIZE_H