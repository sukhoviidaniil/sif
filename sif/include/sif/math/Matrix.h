/***************************************************************
* Author:           <your name>
* Email:            <your email>
* Created:          2026-07-06
*
* License:
*       (c) 2026 <your name>. All rights reserved.
***************************************************************/
#ifndef MATRIX_H
#define MATRIX_H

#include <cstddef>
#include <vector>
#include <iostream>
#include <initializer_list>
#include <stdexcept>

#include "Vector.h"
namespace sif::math {
    /**
     * @brief Generic mathematical matrix with dynamic dimensions.
     *
     * Provides full arithmetic operator support, element access, row/column
     * and sub-matrix extraction, and interoperability with Vector<T> (see
     * Vector.h). T is expected to support +, -, *, / and comparison.
     */
    template <typename T>
    class Matrix
    {
    public:
        // ========== Construction / destruction ==========

        /**
         * @brief Creates an empty matrix (0 rows, 0 columns).
         */
        Matrix();

        /**
         * @brief Creates a matrix with the given dimensions, elements default constructed.
         *
         * @param rows Number of rows.
         * @param cols Number of columns.
         */
        Matrix(size_t rows, size_t cols);

        /**
         * @brief Creates a matrix with the given dimensions, all elements set to fillValue.
         *
         * @param rows Number of rows.
         * @param cols Number of columns.
         * @param fillValue Value assigned to every element.
         */
        Matrix(size_t rows, size_t cols, const T& fillValue);

        /**
         * @brief Creates a matrix from nested brace lists, e.g. Matrix<int> m{ {1,2,3}, {4,5,6} }.
         *
         * All inner lists must have the same length.
         *
         * @param values Initial rows of the matrix.
         */
        Matrix(std::initializer_list<std::initializer_list<T>> values);

        /**
         * @brief Copy constructor.
         */
        Matrix(const Matrix<T>& other);

        /**
         * @brief Move constructor.
         */
        Matrix(Matrix<T>&& other) noexcept;

        /**
         * @brief Destructor.
         */
        ~Matrix();

        // ========== Assignment ==========

        Matrix<T>& operator=(const Matrix<T>& other);
        Matrix<T>& operator=(Matrix<T>&& other) noexcept;

        // ========== Element access ==========

        /**
         * @brief Unchecked access (fast path).
         */
        T& operator()(size_t row, size_t col);
        const T& operator()(size_t row, size_t col) const;

        /**
         * @brief Bounds checked access, throws std::out_of_range on invalid index.
         */
        T& at(size_t row, size_t col);
        const T& at(size_t row, size_t col) const;

        // ========== Size information ==========

        size_t rows() const;
        size_t cols() const;
        bool empty() const;
        bool isSquare() const;

        // ========== Modification ==========

        void resize(size_t newRows, size_t newCols);
        void resize(size_t newRows, size_t newCols, const T& fillValue);
        void clear();

        // ========== Row / column / submatrix access ==========

        /**
         * @brief Returns the given row as a Vector<T>.
         */
        Vector<T> getRow(size_t rowIndex) const;

        /**
         * @brief Returns the given column as a Vector<T>.
         */
        Vector<T> getColumn(size_t colIndex) const;

        /**
         * @brief Replaces the given row with the values from rowVector.
         */
        void setRow(size_t rowIndex, const Vector<T>& rowVector);

        /**
         * @brief Replaces the given column with the values from colVector.
         */
        void setColumn(size_t colIndex, const Vector<T>& colVector);

        /**
         * @brief Returns a rectangular sub-matrix.
         *
         * Throws std::out_of_range if the requested range does not fit
         * inside this matrix.
         *
         * @param rowStart First row of the sub-matrix.
         * @param rowCount Number of rows to copy.
         * @param colStart First column of the sub-matrix.
         * @param colCount Number of columns to copy.
         */
        Matrix<T> subMatrix(size_t rowStart, size_t rowCount,
                             size_t colStart, size_t colCount) const;

        // ========== Matrix <-> Matrix arithmetic ==========

        Matrix<T> operator+(const Matrix<T>& other) const;
        Matrix<T> operator-(const Matrix<T>& other) const;
        Matrix<T>& operator+=(const Matrix<T>& other);
        Matrix<T>& operator-=(const Matrix<T>& other);

        /**
         * @brief Unary minus (negation of every element).
         */
        Matrix<T> operator-() const;

        /**
         * @brief Standard matrix multiplication (this.cols() must equal other.rows()).
         */
        Matrix<T> operator*(const Matrix<T>& other) const;
        Matrix<T>& operator*=(const Matrix<T>& other);

        /**
         * @brief Elementwise multiplication (Hadamard product), requires equal dimensions.
         */
        Matrix<T> hadamard(const Matrix<T>& other) const;

        // ========== Matrix <-> scalar arithmetic ==========

        Matrix<T> operator*(const T& scalar) const;
        Matrix<T> operator/(const T& scalar) const;
        Matrix<T>& operator*=(const T& scalar);
        Matrix<T>& operator/=(const T& scalar);

        // ========== Matrix <-> Vector interoperability ==========

        /**
         * @brief Matrix * column-vector -> vector. Requires cols() == vec.size().
         */
        Vector<T> operator*(const Vector<T>& vec) const;

        /**
         * @brief Builds a matrix from a vector interpreted as a single column (Nx1).
         */
        static Matrix<T> fromColumnVector(const Vector<T>& vec);

        /**
         * @brief Builds a matrix from a vector interpreted as a single row (1xN).
         */
        static Matrix<T> fromRowVector(const Vector<T>& vec);

        // ========== Transformations ==========

        Matrix<T> transpose() const;

        /**
         * @brief Returns the NxN identity matrix.
         */
        static Matrix<T> identity(size_t n);

        /**
         * @brief Returns the determinant using cofactor expansion.
         *
         * Requires a square matrix; throws std::invalid_argument otherwise.
         */
        T determinant() const;

        /**
         * @brief Returns the inverse using Gauss-Jordan elimination.
         *
         * Requires a square, non-singular matrix; throws std::invalid_argument
         * / std::domain_error otherwise. Meaningful mainly for floating point
         * element types.
         */
        Matrix<T> inverse() const;

        // == Comparison ==========

        bool operator==(const Matrix<T>& other) const;
        bool operator!=(const Matrix<T>& other) const;

        // ========== Stream output ==========

        /**
         * @brief Writes the matrix to an output stream, one row per line.
         */
        template <typename U>
        friend std::ostream& operator<<(std::ostream& os, const Matrix<U>& mat);

        /**
         * @brief Allows writing "scalar * matrix" (scalar on the left side).
         */
        template <typename U>
        friend Matrix<U> operator*(const U& scalar, const Matrix<U>& mat);

        /**
         * @brief Allows writing "row-vector * matrix" -> row-vector.
         */
        template <typename U>
        friend Vector<U> operator*(const Vector<U>& vec, const Matrix<U>& mat);

    private:
        size_t m_rows;
        size_t m_cols;
        std::vector<T> m_data; // row-major storage

        size_t indexOf(size_t row, size_t col) const;

        /**
         * @brief Throws std::invalid_argument if dimensions of this matrix and other differ.
         */
        void checkSameDimensions(const Matrix<T>& other, const char* operationName) const;
    };
}

#endif // MATRIX_H
