/***************************************************************
 * Author:           <your name>
 * Email:            <your email>
 * Created:          2026-07-06
 *
 * License:
 *       (c) 2026 <your name>. All rights reserved.
 ***************************************************************/
/**
 * @brief Implementation of the Matrix<T> class template.
 *
 * Explicit template instantiations are provided at the bottom of this
 * file for the commonly used types (int, float, double). If you need
 * Matrix<T> for another type, add an explicit instantiation line for it.
 */

#include "sif/math/Matrix.h"
#include "sif/internal/Rect.h"
#include <string>

namespace sif::math {
    // ========== Construction / destruction ==========

    template<typename T>
    Matrix<T>::Matrix() : m_rows(0), m_cols(0), m_data() {}

    template<typename T>
    Matrix<T>::Matrix(size_t rows, size_t cols) : m_rows(rows), m_cols(cols), m_data(rows * cols) {}

    template<typename T>
    Matrix<T>::Matrix(size_t rows, size_t cols, const T& fillValue)
        : m_rows(rows), m_cols(cols), m_data(rows * cols, fillValue) {}

    template<typename T>
    Matrix<T>::Matrix(std::initializer_list<std::initializer_list<T>> values)
        : m_rows(values.size()), m_cols(values.size() > 0 ? values.begin()->size() : 0) {
        m_data.reserve(m_rows * m_cols);
        for (const auto& row : values) {
            if (row.size() != m_cols) {
                throw std::invalid_argument("Matrix - all rows must have the same length");
            }
            for (const auto& value : row) {
                m_data.push_back(value);
            }
        }
    }

    template<typename T>
    Matrix<T>::Matrix(const Matrix<T>& other) : m_rows(other.m_rows), m_cols(other.m_cols), m_data(other.m_data) {}

    template<typename T>
    Matrix<T>::Matrix(Matrix<T>&& other) noexcept
        : m_rows(other.m_rows), m_cols(other.m_cols), m_data(std::move(other.m_data)) {
        other.m_rows = 0;
        other.m_cols = 0;
    }

    template<typename T>
    Matrix<T>::~Matrix() {}

    // ========== Assignment ==========

    template<typename T>
    Matrix<T>& Matrix<T>::operator=(const Matrix<T>& other) {
        if (this != &other) {
            m_rows = other.m_rows;
            m_cols = other.m_cols;
            m_data = other.m_data;
        }
        return *this;
    }

    template<typename T>
    Matrix<T>& Matrix<T>::operator=(Matrix<T>&& other) noexcept {
        if (this != &other) {
            m_rows = other.m_rows;
            m_cols = other.m_cols;
            m_data = std::move(other.m_data);
            other.m_rows = 0;
            other.m_cols = 0;
        }
        return *this;
    }

    // ========== Element access ==========

    template<typename T>
    size_t Matrix<T>::indexOf(size_t row, size_t col) const {
        return row * m_cols + col;
    }

    template<typename T>
    T& Matrix<T>::operator()(size_t row, size_t col) {
        return m_data[indexOf(row, col)];
    }

    template<typename T>
    const T& Matrix<T>::operator()(size_t row, size_t col) const {
        return m_data[indexOf(row, col)];
    }

    template<typename T>
    T& Matrix<T>::at(size_t row, size_t col) {
        if (row >= m_rows || col >= m_cols) {
            throw std::out_of_range("Matrix::at - index out of range");
        }
        return m_data[indexOf(row, col)];
    }

    template<typename T>
    const T& Matrix<T>::at(size_t row, size_t col) const {
        if (row >= m_rows || col >= m_cols) {
            throw std::out_of_range("Matrix::at - index out of range");
        }
        return m_data[indexOf(row, col)];
    }

    // ========== Size information ==========

    template<typename T>
    size_t Matrix<T>::rows() const {
        return m_rows;
    }

    template<typename T>
    size_t Matrix<T>::cols() const {
        return m_cols;
    }

    template<typename T>
    bool Matrix<T>::empty() const {
        return m_rows == 0 || m_cols == 0;
    }

    template<typename T>
    bool Matrix<T>::isSquare() const {
        return m_rows == m_cols;
    }

    // ========== Modification ==========

    template<typename T>
    void Matrix<T>::resize(size_t newRows, size_t newCols) {
        m_rows = newRows;
        m_cols = newCols;
        m_data.resize(newRows * newCols);
    }

    template<typename T>
    void Matrix<T>::resize(size_t newRows, size_t newCols, const T& fillValue) {
        m_rows = newRows;
        m_cols = newCols;
        m_data.resize(newRows * newCols, fillValue);
    }

    template<typename T>
    void Matrix<T>::clear() {
        m_rows = 0;
        m_cols = 0;
        m_data.clear();
    }

    // ========== Row / column / submatrix access ==========

    template<typename T>
    Vector<T> Matrix<T>::getRow(size_t rowIndex) const {
        if (rowIndex >= m_rows) {
            throw std::out_of_range("Matrix::getRow - row index out of range");
        }
        Vector<T> result(m_cols);
        for (size_t col = 0; col < m_cols; ++col) {
            result[col] = (*this)(rowIndex, col);
        }
        return result;
    }

    template<typename T>
    Vector<T> Matrix<T>::getColumn(size_t colIndex) const {
        if (colIndex >= m_cols) {
            throw std::out_of_range("Matrix::getColumn - column index out of range");
        }
        Vector<T> result(m_rows);
        for (size_t row = 0; row < m_rows; ++row) {
            result[row] = (*this)(row, colIndex);
        }
        return result;
    }

    template<typename T>
    void Matrix<T>::setRow(size_t rowIndex, const Vector<T>& rowVector) {
        if (rowIndex >= m_rows) {
            throw std::out_of_range("Matrix::setRow - row index out of range");
        }
        if (rowVector.size() != m_cols) {
            throw std::invalid_argument("Matrix::setRow - vector size must match column count");
        }
        for (size_t col = 0; col < m_cols; ++col) {
            (*this)(rowIndex, col) = rowVector[col];
        }
    }

    template<typename T>
    void Matrix<T>::setColumn(size_t colIndex, const Vector<T>& colVector) {
        if (colIndex >= m_cols) {
            throw std::out_of_range("Matrix::setColumn - column index out of range");
        }
        if (colVector.size() != m_rows) {
            throw std::invalid_argument("Matrix::setColumn - vector size must match row count");
        }
        for (size_t row = 0; row < m_rows; ++row) {
            (*this)(row, colIndex) = colVector[row];
        }
    }

    template<typename T>
    Matrix<T> Matrix<T>::subMatrix(size_t rowStart, size_t rowCount, size_t colStart, size_t colCount) const {
        if (rowStart + rowCount > m_rows || colStart + colCount > m_cols) {
            throw std::out_of_range("Matrix::subMatrix - requested range out of bounds");
        }

        Matrix<T> result(rowCount, colCount);
        for (size_t r = 0; r < rowCount; ++r) {
            for (size_t c = 0; c < colCount; ++c) {
                result(r, c) = (*this)(rowStart + r, colStart + c);
            }
        }
        return result;
    }

    // ========== Matrix <-> Matrix arithmetic ==========

    template<typename T>
    void Matrix<T>::checkSameDimensions(const Matrix<T>& other, const char* operationName) const {
        if (m_rows != other.m_rows || m_cols != other.m_cols) {
            throw std::invalid_argument(std::string("Matrix::") + operationName +
                                        " - matrices must have the same dimensions");
        }
    }

    template<typename T>
    Matrix<T> Matrix<T>::operator+(const Matrix<T>& other) const {
        checkSameDimensions(other, "operator+");
        Matrix<T> result(m_rows, m_cols);
        for (size_t i = 0; i < m_data.size(); ++i) {
            result.m_data[i] = m_data[i] + other.m_data[i];
        }
        return result;
    }

    template<typename T>
    Matrix<T> Matrix<T>::operator-(const Matrix<T>& other) const {
        checkSameDimensions(other, "operator-");
        Matrix<T> result(m_rows, m_cols);
        for (size_t i = 0; i < m_data.size(); ++i) {
            result.m_data[i] = m_data[i] - other.m_data[i];
        }
        return result;
    }

    template<typename T>
    Matrix<T>& Matrix<T>::operator+=(const Matrix<T>& other) {
        checkSameDimensions(other, "operator+=");
        for (size_t i = 0; i < m_data.size(); ++i) {
            m_data[i] += other.m_data[i];
        }
        return *this;
    }

    template<typename T>
    Matrix<T>& Matrix<T>::operator-=(const Matrix<T>& other) {
        checkSameDimensions(other, "operator-=");
        for (size_t i = 0; i < m_data.size(); ++i) {
            m_data[i] -= other.m_data[i];
        }
        return *this;
    }

    template<typename T>
    Matrix<T> Matrix<T>::operator-() const {
        Matrix<T> result(m_rows, m_cols);
        for (size_t i = 0; i < m_data.size(); ++i) {
            result.m_data[i] = -m_data[i];
        }
        return result;
    }

    template<typename T>
    Matrix<T> Matrix<T>::operator*(const Matrix<T>& other) const {
        if (m_cols != other.m_rows) {
            throw std::invalid_argument("Matrix::operator* - inner dimensions must match");
        }

        Matrix<T> result(m_rows, other.m_cols, T());
        for (size_t r = 0; r < m_rows; ++r) {
            for (size_t k = 0; k < m_cols; ++k) {
                T leftValue = (*this)(r, k);
                for (size_t c = 0; c < other.m_cols; ++c) {
                    result(r, c) += leftValue * other(k, c);
                }
            }
        }
        return result;
    }

    template<typename T>
    Matrix<T>& Matrix<T>::operator*=(const Matrix<T>& other) {
        *this = *this * other;
        return *this;
    }

    template<typename T>
    Matrix<T> Matrix<T>::hadamard(const Matrix<T>& other) const {
        checkSameDimensions(other, "hadamard");
        Matrix<T> result(m_rows, m_cols);
        for (size_t i = 0; i < m_data.size(); ++i) {
            result.m_data[i] = m_data[i] * other.m_data[i];
        }
        return result;
    }

    // ========== Matrix <-> scalar arithmetic ==========

    template<typename T>
    Matrix<T> Matrix<T>::operator*(const T& scalar) const {
        Matrix<T> result(m_rows, m_cols);
        for (size_t i = 0; i < m_data.size(); ++i) {
            result.m_data[i] = m_data[i] * scalar;
        }
        return result;
    }

    template<typename T>
    Matrix<T> Matrix<T>::operator/(const T& scalar) const {
        if (scalar == T()) {
            throw std::invalid_argument("Matrix::operator/ - division by zero scalar");
        }
        Matrix<T> result(m_rows, m_cols);
        for (size_t i = 0; i < m_data.size(); ++i) {
            result.m_data[i] = m_data[i] / scalar;
        }
        return result;
    }

    template<typename T>
    Matrix<T>& Matrix<T>::operator*=(const T& scalar) {
        for (size_t i = 0; i < m_data.size(); ++i) {
            m_data[i] *= scalar;
        }
        return *this;
    }

    template<typename T>
    Matrix<T>& Matrix<T>::operator/=(const T& scalar) {
        if (scalar == T()) {
            throw std::invalid_argument("Matrix::operator/= - division by zero scalar");
        }
        for (size_t i = 0; i < m_data.size(); ++i) {
            m_data[i] /= scalar;
        }
        return *this;
    }

    // ========== Matrix <-> Vector interoperability ==========

    template<typename T>
    Vector<T> Matrix<T>::operator*(const Vector<T>& vec) const {
        if (m_cols != vec.size()) {
            throw std::invalid_argument("Matrix::operator* - matrix column count must match vector size");
        }

        Vector<T> result(m_rows, T());
        for (size_t r = 0; r < m_rows; ++r) {
            T sum = T();
            for (size_t c = 0; c < m_cols; ++c) {
                sum += (*this)(r, c) * vec[c];
            }
            result[r] = sum;
        }
        return result;
    }

    template<typename T>
    Matrix<T> Matrix<T>::fromColumnVector(const Vector<T>& vec) {
        return vec.toColumnMatrix();
    }

    template<typename T>
    Matrix<T> Matrix<T>::fromRowVector(const Vector<T>& vec) {
        return vec.toRowMatrix();
    }

    // ========== Transformations ==========

    template<typename T>
    Matrix<T> Matrix<T>::transpose() const {
        Matrix<T> result(m_cols, m_rows);
        for (size_t r = 0; r < m_rows; ++r) {
            for (size_t c = 0; c < m_cols; ++c) {
                result(c, r) = (*this)(r, c);
            }
        }
        return result;
    }

    template<typename T>
    Matrix<T> Matrix<T>::identity(size_t n) {
        Matrix<T> result(n, n, T());
        for (size_t i = 0; i < n; ++i) {
            result(i, i) = T(1);
        }
        return result;
    }

    template<typename T>
    T Matrix<T>::determinant() const {
        if (!isSquare()) {
            throw std::invalid_argument("Matrix::determinant - matrix must be square");
        }

        size_t n = m_rows;
        if (n == 0) {
            return T(1); // determinant of empty matrix is conventionally 1
        }
        if (n == 1) {
            return (*this)(0, 0);
        }
        if (n == 2) {
            return (*this)(0, 0) * (*this)(1, 1) - (*this)(0, 1) * (*this)(1, 0);
        }

        // Cofactor expansion along the first row.
        T det = T();
        for (size_t col = 0; col < n; ++col) {
            Matrix<T> minor(n - 1, n - 1);
            for (size_t r = 1; r < n; ++r) {
                size_t minorCol = 0;
                for (size_t c = 0; c < n; ++c) {
                    if (c == col) {
                        continue;
                    }
                    minor(r - 1, minorCol) = (*this)(r, c);
                    ++minorCol;
                }
            }

            T cofactor = (*this)(0, col) * minor.determinant();
            if (col % 2 == 0) {
                det += cofactor;
            } else {
                det -= cofactor;
            }
        }
        return det;
    }

    template<typename T>
    Matrix<T> Matrix<T>::inverse() const {
        if (!isSquare()) {
            throw std::invalid_argument("Matrix::inverse - matrix must be square");
        }

        size_t n = m_rows;

        // Build an augmented matrix [ A | I ].
        Matrix<T> augmented(n, 2 * n, T());
        for (size_t r = 0; r < n; ++r) {
            for (size_t c = 0; c < n; ++c) {
                augmented(r, c) = (*this)(r, c);
            }
            augmented(r, n + r) = T(1);
        }

        // Gauss-Jordan elimination with partial pivoting.
        for (size_t pivot = 0; pivot < n; ++pivot) {
            size_t pivotRow = pivot;
            for (size_t r = pivot + 1; r < n; ++r) {
                if (augmented(r, pivot) > augmented(pivotRow, pivot) ||
                    augmented(r, pivot) < -augmented(pivotRow, pivot)) {
                    // Prefer the row with the larger magnitude pivot element.
                    if ((augmented(r, pivot) < T() ? -augmented(r, pivot) : augmented(r, pivot)) >
                        (augmented(pivotRow, pivot) < T() ? -augmented(pivotRow, pivot) : augmented(pivotRow, pivot))) {
                        pivotRow = r;
                    }
                }
            }

            if (augmented(pivotRow, pivot) == T()) {
                throw std::domain_error("Matrix::inverse - matrix is singular and cannot be inverted");
            }

            if (pivotRow != pivot) {
                for (size_t c = 0; c < 2 * n; ++c) {
                    std::swap(augmented(pivot, c), augmented(pivotRow, c));
                }
            }

            T pivotValue = augmented(pivot, pivot);
            for (size_t c = 0; c < 2 * n; ++c) {
                augmented(pivot, c) /= pivotValue;
            }

            for (size_t r = 0; r < n; ++r) {
                if (r == pivot) {
                    continue;
                }
                T factor = augmented(r, pivot);
                for (size_t c = 0; c < 2 * n; ++c) {
                    augmented(r, c) -= factor * augmented(pivot, c);
                }
            }
        }

        Matrix<T> result(n, n);
        for (size_t r = 0; r < n; ++r) {
            for (size_t c = 0; c < n; ++c) {
                result(r, c) = augmented(r, n + c);
            }
        }
        return result;
    }

    // ========== Comparison ==========

    template<typename T>
    bool Matrix<T>::operator==(const Matrix<T>& other) const {
        return m_rows == other.m_rows && m_cols == other.m_cols && m_data == other.m_data;
    }

    template<typename T>
    bool Matrix<T>::operator!=(const Matrix<T>& other) const {
        return !(*this == other);
    }

    // ========== Free functions ==========

    template<typename U>
    std::ostream& operator<<(std::ostream& os, const Matrix<U>& mat) {
        for (size_t r = 0; r < mat.m_rows; ++r) {
            os << "[";
            for (size_t c = 0; c < mat.m_cols; ++c) {
                os << mat(r, c);
                if (c + 1 < mat.m_cols) {
                    os << ", ";
                }
            }
            os << "]";
            if (r + 1 < mat.m_rows) {
                os << "\n";
            }
        }
        return os;
    }

    template<typename U>
    Matrix<U> operator*(const U& scalar, const Matrix<U>& mat) {
        return mat * scalar;
    }

    template<typename U>
    Vector<U> operator*(const Vector<U>& vec, const Matrix<U>& mat) {
        if (vec.size() != mat.rows()) {
            throw std::invalid_argument("operator* - vector size must match matrix row count");
        }

        Vector<U> result(mat.cols(), U());
        for (size_t c = 0; c < mat.cols(); ++c) {
            U sum = U();
            for (size_t r = 0; r < mat.rows(); ++r) {
                sum += vec[r] * mat(r, c);
            }
            result[c] = sum;
        }
        return result;
    }

    // ========== Explicit template instantiations ==========
    // Add a line here for every type you need Matrix<T> to support.

    template class Matrix<int>;
    template class Matrix<float>;
    template class Matrix<double>;

    template std::ostream& operator<<(std::ostream&, const Matrix<int>&);
    template std::ostream& operator<<(std::ostream&, const Matrix<float>&);
    template std::ostream& operator<<(std::ostream&, const Matrix<double>&);

    template Matrix<int> operator*(const int&, const Matrix<int>&);
    template Matrix<float> operator*(const float&, const Matrix<float>&);
    template Matrix<double> operator*(const double&, const Matrix<double>&);

    template Vector<int> operator*(const Vector<int>&, const Matrix<int>&);
    template Vector<float> operator*(const Vector<float>&, const Matrix<float>&);
    template Vector<double> operator*(const Vector<double>&, const Matrix<double>&);

    // sif::intrnl::Rect now supports componentwise +, -, *, / (see
    // Rect.h), so Matrix<Rect> can support the same broad surface as
    // the numeric instantiations above - EXCEPT identity(),
    // determinant(), and inverse(), which all need to construct "T(1)"
    // (a single scalar literal converted to T). Rect has no
    // single-float constructor (only default, and the 4-float
    // x/y/width/height constructor), and there is no reasonable
    // definition of "the rectangle 1" - so those three methods are
    // intentionally never instantiated for Rect. Every other method
    // used by asset::SpriteGrid plus the general arithmetic set is
    // instantiated individually below (rather than
    // "template class Matrix<Rect>;", which would also try - and fail
    // - to instantiate identity() / determinant() / inverse()).
    template Matrix<sif::intrnl::Rect>::Matrix();
    template Matrix<sif::intrnl::Rect>::Matrix(size_t, size_t);
    template Matrix<sif::intrnl::Rect>::Matrix(size_t, size_t, const sif::intrnl::Rect&);
    template Matrix<sif::intrnl::Rect>::Matrix(std::initializer_list<std::initializer_list<sif::intrnl::Rect>>);
    template Matrix<sif::intrnl::Rect>::Matrix(const Matrix<sif::intrnl::Rect>&);
    template Matrix<sif::intrnl::Rect>::Matrix(Matrix<sif::intrnl::Rect>&&) noexcept;
    template Matrix<sif::intrnl::Rect>::~Matrix();
    template Matrix<sif::intrnl::Rect>& Matrix<sif::intrnl::Rect>::operator=(const Matrix<sif::intrnl::Rect>&);
    template Matrix<sif::intrnl::Rect>& Matrix<sif::intrnl::Rect>::operator=(Matrix<sif::intrnl::Rect>&&) noexcept;

    template sif::intrnl::Rect& Matrix<sif::intrnl::Rect>::operator()(size_t, size_t);
    template const sif::intrnl::Rect& Matrix<sif::intrnl::Rect>::operator()(size_t, size_t) const;
    template sif::intrnl::Rect& Matrix<sif::intrnl::Rect>::at(size_t, size_t);
    template const sif::intrnl::Rect& Matrix<sif::intrnl::Rect>::at(size_t, size_t) const;
    template size_t Matrix<sif::intrnl::Rect>::rows() const;
    template size_t Matrix<sif::intrnl::Rect>::cols() const;
    template bool Matrix<sif::intrnl::Rect>::empty() const;
    template bool Matrix<sif::intrnl::Rect>::isSquare() const;

    template void Matrix<sif::intrnl::Rect>::resize(size_t, size_t);
    template void Matrix<sif::intrnl::Rect>::resize(size_t, size_t, const sif::intrnl::Rect&);
    template void Matrix<sif::intrnl::Rect>::clear();

    template Vector<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::getRow(size_t) const;
    template Vector<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::getColumn(size_t) const;
    template void Matrix<sif::intrnl::Rect>::setRow(size_t, const Vector<sif::intrnl::Rect>&);
    template void Matrix<sif::intrnl::Rect>::setColumn(size_t, const Vector<sif::intrnl::Rect>&);
    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::subMatrix(size_t, size_t, size_t, size_t) const;

    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::operator+(const Matrix<sif::intrnl::Rect>&) const;
    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::operator-(const Matrix<sif::intrnl::Rect>&) const;
    template Matrix<sif::intrnl::Rect>& Matrix<sif::intrnl::Rect>::operator+=(const Matrix<sif::intrnl::Rect>&);
    template Matrix<sif::intrnl::Rect>& Matrix<sif::intrnl::Rect>::operator-=(const Matrix<sif::intrnl::Rect>&);
    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::operator-() const;
    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::operator*(const Matrix<sif::intrnl::Rect>&) const;
    template Matrix<sif::intrnl::Rect>& Matrix<sif::intrnl::Rect>::operator*=(const Matrix<sif::intrnl::Rect>&);
    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::hadamard(const Matrix<sif::intrnl::Rect>&) const;

    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::operator*(const sif::intrnl::Rect&) const;
    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::operator/(const sif::intrnl::Rect&) const;
    template Matrix<sif::intrnl::Rect>& Matrix<sif::intrnl::Rect>::operator*=(const sif::intrnl::Rect&);
    template Matrix<sif::intrnl::Rect>& Matrix<sif::intrnl::Rect>::operator/=(const sif::intrnl::Rect&);

    template Vector<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::operator*(const Vector<sif::intrnl::Rect>&) const;
    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::fromColumnVector(const Vector<sif::intrnl::Rect>&);
    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::fromRowVector(const Vector<sif::intrnl::Rect>&);

    template Matrix<sif::intrnl::Rect> Matrix<sif::intrnl::Rect>::transpose() const;

    template bool Matrix<sif::intrnl::Rect>::operator==(const Matrix<sif::intrnl::Rect>&) const;
    template bool Matrix<sif::intrnl::Rect>::operator!=(const Matrix<sif::intrnl::Rect>&) const;

    template std::ostream& operator<<(std::ostream&, const Matrix<sif::intrnl::Rect>&);
    template Matrix<sif::intrnl::Rect> operator*(const sif::intrnl::Rect&, const Matrix<sif::intrnl::Rect>&);
    template Vector<sif::intrnl::Rect> operator*(const Vector<sif::intrnl::Rect>&, const Matrix<sif::intrnl::Rect>&);

} // namespace sif::math