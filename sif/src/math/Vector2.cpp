/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-13
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/math/Vector2.h"
#include <cmath>
#include <string>

namespace sif::math {

    Point2 Vector2::to_Point2() const {
        return {x, y};
    }

    // ========== Arithmetic operators ==========
    Vector2 Vector2::operator+(const Vector2& other) const {
        return {x + other.x, y + other.y};
    }

    Vector2 Vector2::operator-(const Vector2& other) const {
        return {x - other.x, y - other.y};
    }

    Vector2 Vector2::operator*(const Vector2& other) const {
        return {x * other.x, y * other.y};
    }

    Vector2 Vector2::operator/(const Vector2& other) const {
        return {x / other.x, y / other.y};
    }

    Vector2 Vector2::operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }

    Vector2 Vector2::operator/(float scalar) const {
        return {x / scalar, y / scalar};
    }

    // ========== Compound Operators ==========

    Vector2& Vector2::operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2& Vector2::operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2& Vector2::operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2& Vector2::operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // == Comparison ==========

    bool Vector2::operator==(const Vector2& other) const {
        constexpr double EPS = 1e-9;
        return std::fabs(x - other.x) < EPS && std::fabs(y - other.y) < EPS;
    }

    bool Vector2::is_between(const Vector2& start, const Vector2& end) const {
        // Vector from start to end
        const Vector2 segVec = end - start;
        // Vector from start to current point
        const Vector2 pointVec = *this - start;

        // Checking collinearity using cross product (should be close to 0)
        double crossProd = segVec.cross(pointVec);
        if (std::abs(crossProd) > 1e-8) // small error for double
            return false;

        // Check that the point lies between start and end on x and y
        const float dotProd = segVec.dot(pointVec);
        if (dotProd < 0)
            return false; // “before” start
        if (dotProd > segVec.dot(segVec))
            return false; // “after” end
        return true;      // point on a segment
    }

    bool Vector2::is_within_radius(const Vector2& other, const float radius) const {
        // Difference in coordinates
        const float dx = other.x - x;
        const float dy = other.y - y;

        // Square of the distance between points
        const float distanceSquared = dx * dx + dy * dy;

        // Compare with the square of the radius
        return distanceSquared <= radius * radius;
    }

    bool Vector2::has_same_direction(const Vector2& other) const {
        float len1 = length();
        float len2 = other.length();
        if (len1 == 0.0f || len2 == 0.0f)
            return false;

        float cos_angle = dot(other) / (len1 * len2);

        // We consider directions to be identical if the angle between them is approximately 0.
        return cos_angle > 0.999999f;
    }

    // ========== Output to stream ==========

    std::ostream& operator<<(std::ostream& os, const Vector2& vector) {
        const std::string out = "Vector2(" + std::to_string(vector.x) + ", " + std::to_string(vector.y) + ")";
        os << out;
        return os;
    }

    // ========== Vector Operations ==========

    void Vector2::normalize() {
        float len = length();
        constexpr float EPS = 1e-8f;
        if (len > EPS) {
            x /= len;
            y /= len;
        } else {
            x = 0.0f;
            y = 0.0f;
        }
    }

    Vector2 Vector2::normalized() const {
        const float len = length();
        constexpr float EPS = 1e-8f;
        float cx = x, cy = y;
        if (len > EPS) {
            cx /= len;
            cy /= len;
        } else {
            cx = 0.0f;
            cy = 0.0f;
        }
        return {cx, cy};
    }

    float Vector2::length() const {
        return std::sqrt(x * x + y * y);
    }

    float Vector2::dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    // For 2D vectors, cross typically returns a pseudoscalar (the Z-component of the 3D cross product)
    float Vector2::cross(const Vector2& other) const {
        return x * other.y - y * other.x;
    }
} // namespace sif::math
