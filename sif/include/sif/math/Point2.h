/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-13
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_POINT2_H
#define RENDER_ENGINE_POINT2_H

#include <iosfwd>

namespace sif::math {
    /**
     * @brief 2D point or vector with floating-point coordinates.
     *
     * Supports basic arithmetic operations, comparisons, and stream output.
     */
    struct Point2 {
        float x=0.0, y=0.0;

        /**
         * @brief Default constructor initializing point to (0, 0).
         */
        [[nodiscard]] constexpr Point2();

        /**
         * @brief Constructs a point with specified coordinates.
         *
         * @param x X-coordinate
         * @param y Y-coordinate
         */
        [[nodiscard]] constexpr Point2(const float in_x, const float in_y): x(in_x), y(in_y) {
        }
        // ========== Arithmetic operators ==========

        /**
         * @brief Scales the point by a scalar.
         */
        Point2 operator*(float scalar) const;

        /**
         * @brief Divides the point by a scalar.
         */
        Point2 operator/(float scalar) const;

        /**
         * @brief Adds two points component-wise.
         */
        Point2 operator+(const Point2& other) const;

        /**
         * @brief Subtracts two points component-wise.
         */
        Point2 operator-(const Point2& other) const;

        /**
         * @brief Component-wise multiplication with another point.
         */
        Point2 operator*(const Point2& other) const;

        /**
         * @brief Component-wise division by another point.
         */
        Point2 operator/(const Point2& other) const;

        // ========== Compound Operators ==========

        /**
         * @brief Scales the point by a scalar in-place.
         */
        Point2& operator*=(float scalar);

        /**
         * @brief Divides the point by a scalar in-place.
         */
        Point2& operator/=(float scalar);

        /**
         * @brief Adds another point to this point in-place.
         */
        Point2& operator+=(const Point2& other);

        /**
         * @brief Subtracts another point from this point in-place.
         */
        Point2& operator-=(const Point2& other);

        // == Comparison ==========

        /**
         * @brief Compares two points for equality.
         */
        bool operator==(const Point2& other) const;

        // ========== Output to stream ==========

        /**
         * @brief Writes the point to an output stream.
         *
         * Format: "(x, y)"
         */
        friend std::ostream& operator<<(std::ostream& os, const Point2& vector);
    };
}

#endif //RENDER_ENGINE_POINT2_H