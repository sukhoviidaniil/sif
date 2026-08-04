/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-19
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/internal/Rect.h"

#include <ostream>

namespace sif::intrnl {
    Rect Rect::operator+(const Rect& other) const {
        return { x + other.x, y + other.y, width + other.width, height + other.height };
    }

    Rect Rect::operator-(const Rect& other) const {
        return { x - other.x, y - other.y, width - other.width, height - other.height };
    }

    Rect Rect::operator-() const {
        return { -x, -y, -width, -height };
    }

    Rect Rect::operator*(const Rect& other) const {
        return { x * other.x, y * other.y, width * other.width, height * other.height };
    }

    Rect Rect::operator/(const Rect& other) const {
        return { x / other.x, y / other.y, width / other.width, height / other.height };
    }

    Rect Rect::operator*(const float scalar) const {
        return { x * scalar, y * scalar, width * scalar, height * scalar };
    }

    Rect Rect::operator/(const float scalar) const {
        return { x / scalar, y / scalar, width / scalar, height / scalar };
    }

    Rect& Rect::operator+=(const Rect& other) {
        x += other.x;
        y += other.y;
        width += other.width;
        height += other.height;
        return *this;
    }

    Rect& Rect::operator-=(const Rect& other) {
        x -= other.x;
        y -= other.y;
        width -= other.width;
        height -= other.height;
        return *this;
    }

    Rect& Rect::operator*=(const Rect& other) {
        x *= other.x;
        y *= other.y;
        width *= other.width;
        height *= other.height;
        return *this;
    }

    Rect& Rect::operator/=(const Rect& other) {
        x /= other.x;
        y /= other.y;
        width /= other.width;
        height /= other.height;
        return *this;
    }

    Rect& Rect::operator*=(const float scalar) {
        x *= scalar;
        y *= scalar;
        width *= scalar;
        height *= scalar;
        return *this;
    }

    Rect& Rect::operator/=(const float scalar) {
        x /= scalar;
        y /= scalar;
        width /= scalar;
        height /= scalar;
        return *this;
    }

    bool Rect::operator==(const Rect& other) const {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    bool Rect::operator!=(const Rect& other) const {
        return !(*this == other);
    }

    std::ostream& operator<<(std::ostream& os, const Rect& rect) {
        os << "(" << rect.x << ", " << rect.y << ", " << rect.width << ", " << rect.height << ")";
        return os;
    }
}
