/***************************************************************
 * Project:          Render_Engine
 * File:             Vector2.h
 *
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-10-23
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_VECTOR2_H
#define RENDER_ENGINE_VECTOR2_H

#include "Point2.h"
#include <functional>

namespace sif::math {
    /**
     * @brief 2D vector class with floating-point coordinates.
     *
     * Supports arithmetic, comparisons, geometric operations,
     * and conversion to Point2.
     */
    struct Vector2 {
        float x = 0.0, y = 0.0;

        /**
         * @brief Default constructor initializes vector to (0, 0).
         */
        [[nodiscard]] constexpr Vector2() = default;

        /**
         * @brief Constructs a vector with specified coordinates.
         *
         * @param x X-coordinate
         * @param y Y-coordinate
         */
        [[nodiscard]] constexpr Vector2(const float in_x, const float in_y) : x(in_x), y(in_y) {}

        /**
         * @brief Constructs a vector from a Point2.
         */
        [[nodiscard]] explicit constexpr Vector2(const Point2& p) : x(p.x), y(p.y) {}

        /**
         * @brief Converts this vector to a Point2.
         */
        [[nodiscard]] Point2 to_Point2() const;

        // ========== Arithmetic operators ==========
        Vector2 operator+(const Vector2& other) const;
        Vector2 operator-(const Vector2& other) const;
        Vector2 operator*(const Vector2& other) const;
        Vector2 operator/(const Vector2& other) const;
        Vector2 operator*(float scalar) const;
        Vector2 operator/(float scalar) const;

        // ========== Compound Operators ==========
        Vector2& operator+=(const Vector2& other);
        Vector2& operator-=(const Vector2& other);
        Vector2& operator*=(float scalar);
        Vector2& operator/=(float scalar);

        // == Comparison ==========
        bool operator==(const Vector2& other) const;

        /**
         * @brief Checks whether this vector lies between two other vectors.
         *
         * @param start Start vector
         * @param end End vector
         */
        [[nodiscard]] bool is_between(const Vector2& start, const Vector2& end) const;

        /**
         * @brief Checks if another vector lies within a given radius.
         *
         * @param other Vector to test
         * @param radius Radius threshold
         */
        [[nodiscard]] bool is_within_radius(const Vector2& other, float radius) const;

        /**
         * @brief Checks whether this vector has the same direction as another.
         */
        [[nodiscard]] bool has_same_direction(const Vector2& other) const;

        // ========== Output to stream ==========

        /**
         * @brief Writes the vector to an output stream.
         */
        friend std::ostream& operator<<(std::ostream& os, const Vector2& vector);

        // ========== Vector Operations ==========

        /**
         * @brief Normalizes this vector in-place.
         */
        void normalize();

        /**
         * @brief Returns a normalized copy of this vector.
         */
        [[nodiscard]] Vector2 normalized() const;

        /**
         * @brief Returns the length (magnitude) of the vector.
         */
        [[nodiscard]] float length() const;

        /**
         * @brief Computes the dot product with another vector.
         */
        [[nodiscard]] float dot(const Vector2& other) const;

        /**
         * @brief Computes the 2D cross product (scalar) with another vector.
         */
        [[nodiscard]] float cross(const Vector2& other) const;
    };

    /**
     * @brief Hash function for Vector2 to use in unordered containers.
     */
    struct Vector2Hash {
        std::size_t operator()(const Vector2& v) const noexcept {
            return std::hash<int>{}(static_cast<int>(v.x)) ^ std::hash<int>{}(static_cast<int>(v.y)) << 1;
        }
    };
} // namespace sif::math

#endif // RENDER_ENGINE_VECTOR2_H
