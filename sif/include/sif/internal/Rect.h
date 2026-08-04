/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-19
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_RECT_H
#define RENDER_ENGINE_RECT_H

#include <iosfwd>

namespace sif::intrnl{
    /**
     * @brief Axis-aligned rectangle in layout coordinates.
     *
     * Used internally by the layout engine to represent the final
     * position and size of UI elements, and as the element type of
     * math::Vector<Rect> / math::Matrix<Rect> (see asset::SpriteAtlas /
     * asset::SpriteGrid / asset::PrimitiveAnimation), which is why it
     * supports componentwise arithmetic below.
     */
    struct Rect {
        float x = 0; ///< X position (left)
        float y = 0; ///< Y position (top)
        float width = 0; ///< Width
        float height = 0; ///< Height

        Rect() = default;
        Rect(const float left, const float top, const float w, const float h)
            : x(left), y(top), width(w), height(h) {}

        // ========== Arithmetic operators ==========

        /**
         * @brief Componentwise addition (x, y, width, height added independently).
         */
        Rect operator+(const Rect& other) const;

        /**
         * @brief Componentwise subtraction.
         */
        Rect operator-(const Rect& other) const;

        /**
         * @brief Componentwise negation.
         */
        Rect operator-() const;

        /**
         * @brief Componentwise multiplication (x*x, y*y, width*width, height*height).
         *
         * Exists mainly so Rect satisfies math::Vector<T>/math::Matrix<T>,
         * whose scalar-multiply operators take a T rather than a plain
         * float (see operator*(float) for the usual, practical scaling).
         */
        Rect operator*(const Rect& other) const;

        /**
         * @brief Componentwise division.
         */
        Rect operator/(const Rect& other) const;

        /**
         * @brief Scales x, y, width, and height by scalar.
         */
        Rect operator*(float scalar) const;

        /**
         * @brief Divides x, y, width, and height by scalar.
         */
        Rect operator/(float scalar) const;

        // ========== Compound operators ==========

        Rect& operator+=(const Rect& other);
        Rect& operator-=(const Rect& other);
        Rect& operator*=(const Rect& other);
        Rect& operator/=(const Rect& other);
        Rect& operator*=(float scalar);
        Rect& operator/=(float scalar);

        // ========== Comparison ==========

        bool operator==(const Rect& other) const;
        bool operator!=(const Rect& other) const;

        // ========== Output to stream ==========

        /**
         * @brief Writes the rect to an output stream.
         *
         * Format: "(x, y, width, height)"
         */
        friend std::ostream& operator<<(std::ostream& os, const Rect& rect);
    };
}
#endif //RENDER_ENGINE_RECT_H