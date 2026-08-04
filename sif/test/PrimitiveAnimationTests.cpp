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
#include "Fixtures.h"

using namespace sif;
using namespace sif::test::fixtures;

namespace {
    math::Vector<intrnl::Rect> three_frames() {
        math::Vector<intrnl::Rect> frames(3);
        frames[0] = intrnl::Rect(0, 0, 10, 10);
        frames[1] = intrnl::Rect(10, 0, 10, 10);
        frames[2] = intrnl::Rect(20, 0, 10, 10);
        return frames;
    }
}

SIF_TEST(looping_animation_advances_and_wraps) {
    const FakePrimitiveAnimation anim(three_frames(), 0.1f, /*loop=*/true);

    SIF_CHECK(anim.frame_index_at(0.0f) == 0);
    SIF_CHECK(anim.frame_index_at(0.05f) == 0);
    SIF_CHECK(anim.frame_index_at(0.15f) == 1);
    SIF_CHECK(anim.frame_index_at(0.25f) == 2);
    SIF_CHECK(anim.frame_index_at(0.35f) == 0); // wraps after 0.3s total duration
}

SIF_TEST(non_looping_animation_holds_on_last_frame) {
    const FakePrimitiveAnimation anim(three_frames(), 0.1f, /*loop=*/false);

    SIF_CHECK(anim.frame_index_at(0.05f) == 0);
    SIF_CHECK(anim.frame_index_at(1.0f) == 2);
    SIF_CHECK(anim.frame_index_at(1000.0f) == 2); // still holds, does not wrap
}

SIF_TEST(animation_frame_rect_matches_index) {
    const FakePrimitiveAnimation anim(three_frames(), 0.1f, true);
    SIF_CHECK(anim.frame_at(0.15f) == anim.rect_of(1));
}

SIF_TEST(empty_animation_defaults_to_frame_zero_without_throwing) {
    const FakePrimitiveAnimation anim(math::Vector<intrnl::Rect>(0), 0.1f, true);
    SIF_CHECK(anim.frame_count() == 0);
    SIF_CHECK(anim.frame_index_at(1.0f) == 0);
}
