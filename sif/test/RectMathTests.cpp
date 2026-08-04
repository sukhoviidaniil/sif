/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"

#include "sif/internal/Rect.h"
#include "sif/math/Vector.h"
#include "sif/math/Matrix.h"

using namespace sif;

SIF_TEST(rect_componentwise_arithmetic) {
    const intrnl::Rect a(1, 2, 10, 20);
    const intrnl::Rect b(3, 4, 5, 6);

    const intrnl::Rect sum = a + b;
    SIF_CHECK(sum == intrnl::Rect(4, 6, 15, 26));

    const intrnl::Rect diff = a - b;
    SIF_CHECK(diff == intrnl::Rect(-2, -2, 5, 14));

    const intrnl::Rect scaled = a * 2.f;
    SIF_CHECK(scaled == intrnl::Rect(2, 4, 20, 40));

    const intrnl::Rect negated = -a;
    SIF_CHECK(negated == intrnl::Rect(-1, -2, -10, -20));

    SIF_CHECK(a != b);
}

SIF_TEST(vector_of_rect_supports_arithmetic_and_indexing) {
    math::Vector<intrnl::Rect> a{intrnl::Rect(0,0,1,1), intrnl::Rect(1,1,2,2)};
    math::Vector<intrnl::Rect> b{intrnl::Rect(1,1,1,1), intrnl::Rect(1,1,1,1)};

    const math::Vector<intrnl::Rect> sum = a + b;
    SIF_CHECK(sum[0] == intrnl::Rect(1,1,2,2));
    SIF_CHECK(sum[1] == intrnl::Rect(2,2,3,3));

    const math::Vector<intrnl::Rect> product = a.hadamard(b);
    SIF_CHECK(product[1] == intrnl::Rect(1,1,2,2));
}

SIF_TEST(matrix_of_rect_supports_arithmetic_and_indexing) {
    math::Matrix<intrnl::Rect> a(2, 2, intrnl::Rect(1,1,1,1));
    math::Matrix<intrnl::Rect> b(2, 2, intrnl::Rect(2,2,2,2));

    const math::Matrix<intrnl::Rect> sum = a + b;
    SIF_CHECK(sum(0, 0) == intrnl::Rect(3,3,3,3));

    const math::Matrix<intrnl::Rect> transposed = sum.transpose();
    SIF_CHECK(transposed.rows() == 2);
    SIF_CHECK(transposed.cols() == 2);
    SIF_CHECK(transposed(0, 1) == sum(1, 0));
}
