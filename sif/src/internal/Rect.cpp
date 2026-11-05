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

#include <algorithm>
#include <ostream>

namespace sif::intrnl {
Rect Rect::operator+(const Rect& other) const {
    return {x + other.x, y + other.y, width + other.width, height + other.height};
}

Rect Rect::operator-(const Rect& other) const {
    return {x - other.x, y - other.y, width - other.width, height - other.height};
}

Rect Rect::operator-() const { return {-x, -y, -width, -height}; }

Rect Rect::operator*(const Rect& other) const {
    return {x * other.x, y * other.y, width * other.width, height * other.height};
}

Rect Rect::operator/(const Rect& other) const {
    return {x / other.x, y / other.y, width / other.width, height / other.height};
}

Rect Rect::operator*(const float scalar) const { return {x * scalar, y * scalar, width * scalar, height * scalar}; }

Rect Rect::operator/(const float scalar) const { return {x / scalar, y / scalar, width / scalar, height / scalar}; }

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

bool Rect::operator!=(const Rect& other) const { return !(*this == other); }

Rect Rect::intersection(const Rect& other) const {
    const float left = std::max(x, other.x);
    const float top = std::max(y, other.y);
    const float right = std::min(x + width, other.x + other.width);
    const float bottom = std::min(y + height, other.y + other.height);
    return {left, top, std::max(0.f, right - left), std::max(0.f, bottom - top)};
}

bool Rect::contains(const float px, const float py) const {
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

std::ostream& operator<<(std::ostream& os, const Rect& rect) {
    os << "(" << rect.x << ", " << rect.y << ", " << rect.width << ", " << rect.height << ")";
    return os;
}
} // namespace sif::intrnl
