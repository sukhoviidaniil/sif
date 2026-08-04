/***************************************************************
* Author:           <your name>
* Email:            <your email>
* Created:          2026-07-06
*
* License:
*       (c) 2026 <your name>. All rights reserved.
***************************************************************/
#ifndef VECTOR_H
#define VECTOR_H

#include <cstddef>
#include <vector>
#include <iostream>
#include <initializer_list>
#include <stdexcept>

namespace sif::math {
    // Forward declaration to allow conversion helpers without circular include.
    template <typename T>
    class Matrix;

    /**
     * @brief Generic mathematical vector with dynamic size.
     *
     * Provides full arithmetic operator support, element access, sub-vector
     * extraction, and conversion helpers so it can interoperate with the
     * Matrix<T> class (see Matrix.h). T is expected to support +, -, *, /
     * and comparison.
     */

    template <typename T>
    class Vector
    {
    public:
        // ========== Construction / destruction ==========

        /**
         * @brief Creates an empty vector (size 0).
         */
        Vector();

        /**
         * @brief Creates a vector of the given size, elements default constructed.
         *
         * @param size Number of elements.
         */
        explicit Vector(size_t size);

        /**
         * @brief Creates a vector of the given size, all elements set to fillValue.
         *
         * @param size Number of elements.
         * @param fillValue Value assigned to every element.
         */
        Vector(size_t size, const T& fillValue);

        /**
         * @brief Creates a vector from a brace initialized list, e.g. Vector<int> v{1,2,3}.
         *
         * @param values Initial elements.
         */
        Vector(std::initializer_list<T> values);

        /**
         * @brief Copy constructor.
         */
        Vector(const Vector<T>& other);

        /**
         * @brief Move constructor.
         */
        Vector(Vector<T>&& other) noexcept;

        /**
         * @brief Destructor.
         */
        ~Vector();

        // ========== Assignment ==========

        Vector<T>& operator=(const Vector<T>& other);
        Vector<T>& operator=(Vector<T>&& other) noexcept;

        // ========== Element access ==========

        /**
         * @brief Unchecked access (fast path).
         */
        T& operator[](size_t index);
        const T& operator[](size_t index) const;

        /**
         * @brief Bounds checked access, throws std::out_of_range on invalid index.
         */
        T& at(size_t index);
        const T& at(size_t index) const;

        /**
         * @brief First and last element helpers.
         */
        T& front();
        const T& front() const;
        T& back();
        const T& back() const;

        // ========== Size / capacity ==========

        size_t size() const;
        bool empty() const;

        // ========== Modification ==========

        void resize(size_t newSize);
        void resize(size_t newSize, const T& fillValue);
        void clear();
        void pushBack(const T& value);

        // ========== Sub-vector extraction ==========

        /**
         * @brief Returns a new vector containing "length" elements starting at startIndex.
         *
         * Throws std::out_of_range if the requested range is outside the
         * bounds of this vector.
         *
         * @param startIndex Index of the first element to copy.
         * @param length Number of elements to copy.
         */
        Vector<T> subVector(size_t startIndex, size_t length) const;

        // ========== Vector <-> Vector arithmetic ==========

        Vector<T> operator+(const Vector<T>& other) const;
        Vector<T> operator-(const Vector<T>& other) const;
        Vector<T>& operator+=(const Vector<T>& other);
        Vector<T>& operator-=(const Vector<T>& other);

        /**
         * @brief Unary minus (negation of every element).
         */
        Vector<T> operator-() const;

        // ========== Vector <-> scalar arithmetic ==========

        Vector<T> operator*(const T& scalar) const;
        Vector<T> operator/(const T& scalar) const;
        Vector<T>& operator*=(const T& scalar);
        Vector<T>& operator/=(const T& scalar);

        // ========== Products ==========

        /**
         * @brief Elementwise multiplication (Hadamard product).
         */
        Vector<T> hadamard(const Vector<T>& other) const;

        /**
         * @brief Dot product (inner product).
         */
        T dot(const Vector<T>& other) const;

        // ========== Norms ==========

        /**
         * @brief Euclidean (L2) norm, computed in double precision.
         */
        double norm() const;

        /**
         * @brief Returns a new vector of type double scaled to unit length.
         */
        Vector<double> normalized() const;

        // == Comparison ==========

        bool operator==(const Vector<T>& other) const;
        bool operator!=(const Vector<T>& other) const;

        // ========== Iterators ==========

        typename std::vector<T>::iterator begin();
        typename std::vector<T>::iterator end();
        typename std::vector<T>::const_iterator begin() const;
        typename std::vector<T>::const_iterator end() const;

        // ========== Conversion to Matrix ==========

        /**
         * @brief Returns this vector as a Nx1 column matrix.
         */
        Matrix<T> toColumnMatrix() const;

        /**
         * @brief Returns this vector as a 1xN row matrix.
         */
        Matrix<T> toRowMatrix() const;

        // ========== Raw data access ==========

        T* data();
        const T* data() const;

        // ========== Free function friends ==========

        /**
         * @brief Writes the vector to an output stream.
         *
         * Format: "[a, b, c]"
         */
        template <typename U>
        friend std::ostream& operator<<(std::ostream& os, const Vector<U>& vec);

        /**
         * @brief Allows writing "scalar * vector" (scalar on the left side).
         */
        template <typename U>
        friend Vector<U> operator*(const U& scalar, const Vector<U>& vec);

    private:
        std::vector<T> m_data;

        /**
         * @brief Throws std::invalid_argument if sizes of this vector and other differ.
         */
        void checkSameSize(const Vector<T>& other, const char* operationName) const;
    };
}
#endif // VECTOR_H
