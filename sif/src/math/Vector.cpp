/***************************************************************
 * Author:           <your name>
 * Email:            <your email>
 * Created:          2026-07-06
 *
 * License:
 *       (c) 2026 <your name>. All rights reserved.
 ***************************************************************/
/**
 * @brief Implementation of the Vector<T> class template.
 *
 * Explicit template instantiations are provided at the bottom of this
 * file for the commonly used types (int, float, double). If you need
 * Vector<T> for another type, add an explicit instantiation line for it.
 */

#include "sif/math/Vector.h"
#include "sif/internal/Rect.h"
#include "sif/math/Matrix.h"
#include <algorithm>
#include <cmath>

namespace sif::math {
    // ========== Construction / destruction ==========

    template<typename T>
    Vector<T>::Vector() : m_data() {}

    template<typename T>
    Vector<T>::Vector(size_t size) : m_data(size) {}

    template<typename T>
    Vector<T>::Vector(size_t size, const T& fillValue) : m_data(size, fillValue) {}

    template<typename T>
    Vector<T>::Vector(std::initializer_list<T> values) : m_data(values) {}

    template<typename T>
    Vector<T>::Vector(const Vector<T>& other) : m_data(other.m_data) {}

    template<typename T>
    Vector<T>::Vector(Vector<T>&& other) noexcept : m_data(std::move(other.m_data)) {}

    template<typename T>
    Vector<T>::~Vector() {}

    // ========== Assignment ==========

    template<typename T>
    Vector<T>& Vector<T>::operator=(const Vector<T>& other) {
        if (this != &other) {
            m_data = other.m_data;
        }
        return *this;
    }

    template<typename T>
    Vector<T>& Vector<T>::operator=(Vector<T>&& other) noexcept {
        if (this != &other) {
            m_data = std::move(other.m_data);
        }
        return *this;
    }

    // ========== Element access ==========

    template<typename T>
    T& Vector<T>::operator[](size_t index) {
        return m_data[index];
    }

    template<typename T>
    const T& Vector<T>::operator[](size_t index) const {
        return m_data[index];
    }

    template<typename T>
    T& Vector<T>::at(size_t index) {
        if (index >= m_data.size()) {
            throw std::out_of_range("Vector::at - index out of range");
        }
        return m_data[index];
    }

    template<typename T>
    const T& Vector<T>::at(size_t index) const {
        if (index >= m_data.size()) {
            throw std::out_of_range("Vector::at - index out of range");
        }
        return m_data[index];
    }

    template<typename T>
    T& Vector<T>::front() {
        return m_data.front();
    }

    template<typename T>
    const T& Vector<T>::front() const {
        return m_data.front();
    }

    template<typename T>
    T& Vector<T>::back() {
        return m_data.back();
    }

    template<typename T>
    const T& Vector<T>::back() const {
        return m_data.back();
    }

    // ========== Size / capacity ==========

    template<typename T>
    size_t Vector<T>::size() const {
        return m_data.size();
    }

    template<typename T>
    bool Vector<T>::empty() const {
        return m_data.empty();
    }

    // ========== Modification ==========

    template<typename T>
    void Vector<T>::resize(size_t newSize) {
        m_data.resize(newSize);
    }

    template<typename T>
    void Vector<T>::resize(size_t newSize, const T& fillValue) {
        m_data.resize(newSize, fillValue);
    }

    template<typename T>
    void Vector<T>::clear() {
        m_data.clear();
    }

    template<typename T>
    void Vector<T>::pushBack(const T& value) {
        m_data.push_back(value);
    }

    // ========== Sub-vector extraction ==========

    template<typename T>
    Vector<T> Vector<T>::subVector(size_t startIndex, size_t length) const {
        if (startIndex + length > m_data.size()) {
            throw std::out_of_range("Vector::subVector - requested range out of bounds");
        }

        Vector<T> result(length);
        for (size_t i = 0; i < length; ++i) {
            result[i] = m_data[startIndex + i];
        }
        return result;
    }

    // ========== Vector <-> Vector arithmetic ==========

    template<typename T>
    void Vector<T>::checkSameSize(const Vector<T>& other, const char* operationName) const {
        if (m_data.size() != other.m_data.size()) {
            throw std::invalid_argument(std::string("Vector::") + operationName + " - vectors must have the same size");
        }
    }

    template<typename T>
    Vector<T> Vector<T>::operator+(const Vector<T>& other) const {
        checkSameSize(other, "operator+");
        Vector<T> result(m_data.size());
        for (size_t i = 0; i < m_data.size(); ++i) {
            result[i] = m_data[i] + other.m_data[i];
        }
        return result;
    }

    template<typename T>
    Vector<T> Vector<T>::operator-(const Vector<T>& other) const {
        checkSameSize(other, "operator-");
        Vector<T> result(m_data.size());
        for (size_t i = 0; i < m_data.size(); ++i) {
            result[i] = m_data[i] - other.m_data[i];
        }
        return result;
    }

    template<typename T>
    Vector<T>& Vector<T>::operator+=(const Vector<T>& other) {
        checkSameSize(other, "operator+=");
        for (size_t i = 0; i < m_data.size(); ++i) {
            m_data[i] += other.m_data[i];
        }
        return *this;
    }

    template<typename T>
    Vector<T>& Vector<T>::operator-=(const Vector<T>& other) {
        checkSameSize(other, "operator-=");
        for (size_t i = 0; i < m_data.size(); ++i) {
            m_data[i] -= other.m_data[i];
        }
        return *this;
    }

    template<typename T>
    Vector<T> Vector<T>::operator-() const {
        Vector<T> result(m_data.size());
        for (size_t i = 0; i < m_data.size(); ++i) {
            result[i] = -m_data[i];
        }
        return result;
    }

    // ========== Vector <-> scalar arithmetic ==========

    template<typename T>
    Vector<T> Vector<T>::operator*(const T& scalar) const {
        Vector<T> result(m_data.size());
        for (size_t i = 0; i < m_data.size(); ++i) {
            result[i] = m_data[i] * scalar;
        }
        return result;
    }

    template<typename T>
    Vector<T> Vector<T>::operator/(const T& scalar) const {
        if (scalar == T()) {
            throw std::invalid_argument("Vector::operator/ - division by zero scalar");
        }
        Vector<T> result(m_data.size());
        for (size_t i = 0; i < m_data.size(); ++i) {
            result[i] = m_data[i] / scalar;
        }
        return result;
    }

    template<typename T>
    Vector<T>& Vector<T>::operator*=(const T& scalar) {
        for (size_t i = 0; i < m_data.size(); ++i) {
            m_data[i] *= scalar;
        }
        return *this;
    }

    template<typename T>
    Vector<T>& Vector<T>::operator/=(const T& scalar) {
        if (scalar == T()) {
            throw std::invalid_argument("Vector::operator/= - division by zero scalar");
        }
        for (size_t i = 0; i < m_data.size(); ++i) {
            m_data[i] /= scalar;
        }
        return *this;
    }

    // ========== Products ==========

    template<typename T>
    Vector<T> Vector<T>::hadamard(const Vector<T>& other) const {
        checkSameSize(other, "hadamard");
        Vector<T> result(m_data.size());
        for (size_t i = 0; i < m_data.size(); ++i) {
            result[i] = m_data[i] * other.m_data[i];
        }
        return result;
    }

    template<typename T>
    T Vector<T>::dot(const Vector<T>& other) const {
        checkSameSize(other, "dot");
        T sum = T();
        for (size_t i = 0; i < m_data.size(); ++i) {
            sum += m_data[i] * other.m_data[i];
        }
        return sum;
    }

    // ========== Norms ==========

    template<typename T>
    double Vector<T>::norm() const {
        double sumOfSquares = 0.0;
        for (size_t i = 0; i < m_data.size(); ++i) {
            double value = static_cast<double>(m_data[i]);
            sumOfSquares += value * value;
        }
        return std::sqrt(sumOfSquares);
    }

    template<typename T>
    Vector<double> Vector<T>::normalized() const {
        double length = norm();
        if (length == 0.0) {
            throw std::domain_error("Vector::normalized - cannot normalize a zero-length vector");
        }

        Vector<double> result(m_data.size());
        for (size_t i = 0; i < m_data.size(); ++i) {
            result[i] = static_cast<double>(m_data[i]) / length;
        }
        return result;
    }

    // ========== Comparison ==========

    template<typename T>
    bool Vector<T>::operator==(const Vector<T>& other) const {
        return m_data == other.m_data;
    }

    template<typename T>
    bool Vector<T>::operator!=(const Vector<T>& other) const {
        return !(*this == other);
    }

    // ========== Iterators ==========

    template<typename T>
    typename std::vector<T>::iterator Vector<T>::begin() {
        return m_data.begin();
    }

    template<typename T>
    typename std::vector<T>::iterator Vector<T>::end() {
        return m_data.end();
    }

    template<typename T>
    typename std::vector<T>::const_iterator Vector<T>::begin() const {
        return m_data.begin();
    }

    template<typename T>
    typename std::vector<T>::const_iterator Vector<T>::end() const {
        return m_data.end();
    }

    // ========== Conversion to Matrix ==========

    template<typename T>
    Matrix<T> Vector<T>::toColumnMatrix() const {
        Matrix<T> result(m_data.size(), 1);
        for (size_t i = 0; i < m_data.size(); ++i) {
            result(i, 0) = m_data[i];
        }
        return result;
    }

    template<typename T>
    Matrix<T> Vector<T>::toRowMatrix() const {
        Matrix<T> result(1, m_data.size());
        for (size_t j = 0; j < m_data.size(); ++j) {
            result(0, j) = m_data[j];
        }
        return result;
    }

    // ========== Raw data access ==========

    template<typename T>
    T* Vector<T>::data() {
        return m_data.data();
    }

    template<typename T>
    const T* Vector<T>::data() const {
        return m_data.data();
    }

    // ========== Free functions ==========

    template<typename U>
    std::ostream& operator<<(std::ostream& os, const Vector<U>& vec) {
        os << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            os << vec[i];
            if (i + 1 < vec.size()) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }

    template<typename U>
    Vector<U> operator*(const U& scalar, const Vector<U>& vec) {
        return vec * scalar;
    }

    // ========== Explicit template instantiations ==========
    // Add a line here for every type you need Vector<T> to support.

    template class Vector<int>;
    template class Vector<float>;
    template class Vector<double>;

    template std::ostream& operator<<(std::ostream&, const Vector<int>&);
    template std::ostream& operator<<(std::ostream&, const Vector<float>&);
    template std::ostream& operator<<(std::ostream&, const Vector<double>&);

    template Vector<int> operator*(const int&, const Vector<int>&);
    template Vector<float> operator*(const float&, const Vector<float>&);
    template Vector<double> operator*(const double&, const Vector<double>&);

    // sif::intrnl::Rect now supports componentwise +, -, *, / (see
    // Rect.h), so Vector<Rect> can support the same broad surface as
    // the numeric instantiations above - EXCEPT norm() / normalized(),
    // which cast each element to double (a rectangle has no single
    // scalar magnitude, so that cast is intentionally not provided).
    // Those two methods are therefore never instantiated for Rect;
    // every other method used by asset::SpriteAtlas / PrimitiveAnimation
    // plus the general arithmetic set is instantiated individually
    // below (rather than "template class Vector<Rect>;", which would
    // also try - and fail - to instantiate norm()/normalized()).
    template Vector<sif::intrnl::Rect>::Vector();
    template Vector<sif::intrnl::Rect>::Vector(size_t);
    template Vector<sif::intrnl::Rect>::Vector(size_t, const sif::intrnl::Rect&);
    template Vector<sif::intrnl::Rect>::Vector(std::initializer_list<sif::intrnl::Rect>);
    template Vector<sif::intrnl::Rect>::Vector(const Vector<sif::intrnl::Rect>&);
    template Vector<sif::intrnl::Rect>::Vector(Vector<sif::intrnl::Rect>&&) noexcept;
    template Vector<sif::intrnl::Rect>::~Vector();
    template Vector<sif::intrnl::Rect>& Vector<sif::intrnl::Rect>::operator=(const Vector<sif::intrnl::Rect>&);
    template Vector<sif::intrnl::Rect>& Vector<sif::intrnl::Rect>::operator=(Vector<sif::intrnl::Rect>&&) noexcept;

    template sif::intrnl::Rect& Vector<sif::intrnl::Rect>::operator[](size_t);
    template const sif::intrnl::Rect& Vector<sif::intrnl::Rect>::operator[](size_t) const;
    template sif::intrnl::Rect& Vector<sif::intrnl::Rect>::at(size_t);
    template const sif::intrnl::Rect& Vector<sif::intrnl::Rect>::at(size_t) const;
    template size_t Vector<sif::intrnl::Rect>::size() const;

    template Vector<sif::intrnl::Rect> Vector<sif::intrnl::Rect>::operator+(const Vector<sif::intrnl::Rect>&) const;
    template Vector<sif::intrnl::Rect> Vector<sif::intrnl::Rect>::operator-(const Vector<sif::intrnl::Rect>&) const;
    template Vector<sif::intrnl::Rect>& Vector<sif::intrnl::Rect>::operator+=(const Vector<sif::intrnl::Rect>&);
    template Vector<sif::intrnl::Rect>& Vector<sif::intrnl::Rect>::operator-=(const Vector<sif::intrnl::Rect>&);
    template Vector<sif::intrnl::Rect> Vector<sif::intrnl::Rect>::operator-() const;

    template Vector<sif::intrnl::Rect> Vector<sif::intrnl::Rect>::operator*(const sif::intrnl::Rect&) const;
    template Vector<sif::intrnl::Rect> Vector<sif::intrnl::Rect>::operator/(const sif::intrnl::Rect&) const;
    template Vector<sif::intrnl::Rect>& Vector<sif::intrnl::Rect>::operator*=(const sif::intrnl::Rect&);
    template Vector<sif::intrnl::Rect>& Vector<sif::intrnl::Rect>::operator/=(const sif::intrnl::Rect&);

    template Vector<sif::intrnl::Rect> Vector<sif::intrnl::Rect>::hadamard(const Vector<sif::intrnl::Rect>&) const;
    template sif::intrnl::Rect Vector<sif::intrnl::Rect>::dot(const Vector<sif::intrnl::Rect>&) const;

    template bool Vector<sif::intrnl::Rect>::operator==(const Vector<sif::intrnl::Rect>&) const;
    template bool Vector<sif::intrnl::Rect>::operator!=(const Vector<sif::intrnl::Rect>&) const;

    template std::vector<sif::intrnl::Rect>::iterator Vector<sif::intrnl::Rect>::begin();
    template std::vector<sif::intrnl::Rect>::iterator Vector<sif::intrnl::Rect>::end();
    template std::vector<sif::intrnl::Rect>::const_iterator Vector<sif::intrnl::Rect>::begin() const;
    template std::vector<sif::intrnl::Rect>::const_iterator Vector<sif::intrnl::Rect>::end() const;

    template std::ostream& operator<<(std::ostream&, const Vector<sif::intrnl::Rect>&);
    template Vector<sif::intrnl::Rect> operator*(const sif::intrnl::Rect&, const Vector<sif::intrnl::Rect>&);

    // Needed by Matrix<Rect>::fromColumnVector / fromRowVector (see Matrix.cpp).
    template Matrix<sif::intrnl::Rect> Vector<sif::intrnl::Rect>::toColumnMatrix() const;
    template Matrix<sif::intrnl::Rect> Vector<sif::intrnl::Rect>::toRowMatrix() const;

} // namespace sif::math