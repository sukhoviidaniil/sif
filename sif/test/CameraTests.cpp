/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-08-04
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"

#include <cmath>
#include <filesystem>
#include <fstream>

#include "sif/io/from_JSON.h"
#include "sif/render/Camera.h"

using namespace sif;

namespace {
    constexpr float eps = 0.001f;

    bool near_eq(const float a, const float b) {
        return std::abs(a - b) < eps;
    }

    bool near_eq(const math::Point2 a, const math::Point2 b) {
        return near_eq(a.x, b.x) && near_eq(a.y, b.y);
    }

    bool near_eq(const intrnl::Rect& a, const intrnl::Rect& b) {
        return near_eq(a.x, b.x) && near_eq(a.y, b.y)
            && near_eq(a.width, b.width) && near_eq(a.height, b.height);
    }
}

SIF_TEST(camera_stretch_maps_the_world_corners_onto_the_viewport_corners) {
    rnd::Camera camera({800.f, 600.f}, rnd::AspectPolicy::Stretch);

    SIF_CHECK(near_eq(camera.world_to_screen({-1.f, -1.f}), math::Point2(0.f, 0.f)));
    SIF_CHECK(near_eq(camera.world_to_screen({1.f, 1.f}), math::Point2(800.f, 600.f)));
    SIF_CHECK(near_eq(camera.world_to_screen({0.f, 0.f}), math::Point2(400.f, 300.f)));
    SIF_CHECK(near_eq(camera.world_to_screen({0.5f, -0.5f}), math::Point2(600.f, 150.f)));
}

SIF_TEST(camera_fit_keeps_the_world_square_and_letterboxes_the_rest) {
    const rnd::Camera camera({800.f, 600.f}, rnd::AspectPolicy::Fit);

    // The short axis decides: 600 / 2 = 300 pixels per world unit.
    SIF_CHECK(near_eq(camera.scale().x, 300.f));
    SIF_CHECK(near_eq(camera.scale().y, 300.f));

    // Vertically the world fills the screen, horizontally it is centred
    // with a 100px bar on each side.
    SIF_CHECK(near_eq(camera.world_to_screen({-1.f, -1.f}), math::Point2(100.f, 0.f)));
    SIF_CHECK(near_eq(camera.world_to_screen({1.f, 1.f}), math::Point2(700.f, 600.f)));
}

SIF_TEST(camera_fill_covers_the_viewport_and_crops_the_long_axis) {
    const rnd::Camera camera({800.f, 600.f}, rnd::AspectPolicy::Fill);

    // The long axis decides: 800 / 2 = 400 pixels per world unit.
    SIF_CHECK(near_eq(camera.scale().x, 400.f));

    // Horizontally exact, vertically the world overflows by 100px per side.
    SIF_CHECK(near_eq(camera.world_to_screen({-1.f, -1.f}), math::Point2(0.f, -100.f)));
    SIF_CHECK(near_eq(camera.world_to_screen({1.f, 1.f}), math::Point2(800.f, 700.f)));
}

SIF_TEST(camera_projection_is_resolution_independent) {
    // The same world point must land on the same *relative* spot no
    // matter how large the window is - that is the whole reason the
    // world is normalized.
    const rnd::Camera small({400.f, 400.f}, rnd::AspectPolicy::Fit);
    const rnd::Camera large({1600.f, 1600.f}, rnd::AspectPolicy::Fit);

    const math::Point2 world{0.25f, -0.5f};
    const math::Point2 a = small.world_to_screen(world);
    const math::Point2 b = large.world_to_screen(world);

    SIF_CHECK(near_eq(a.x / 400.f, b.x / 1600.f));
    SIF_CHECK(near_eq(a.y / 400.f, b.y / 1600.f));
}

SIF_TEST(camera_screen_to_world_is_the_inverse_of_world_to_screen) {
    rnd::Camera camera({1024.f, 768.f}, rnd::AspectPolicy::Fit);
    camera.set_center({0.2f, -0.3f});
    camera.set_zoom(1.7f);

    for (const math::Point2 world : {math::Point2(0.f, 0.f),
                                     math::Point2(-1.f, -1.f),
                                     math::Point2(0.62f, 0.13f)}) {
        const math::Point2 round_trip = camera.screen_to_world(camera.world_to_screen(world));
        SIF_CHECK(near_eq(round_trip, world));
    }
}

SIF_TEST(camera_center_moves_the_world_not_the_camera_frame) {
    rnd::Camera camera({800.f, 800.f}, rnd::AspectPolicy::Fit);
    camera.set_center({0.5f, 0.f});

    // Whatever is at the centre must land in the middle of the viewport.
    SIF_CHECK(near_eq(camera.world_to_screen({0.5f, 0.f}), math::Point2(400.f, 400.f)));
    // And the origin has moved left by 0.5 world units * 400 px/unit.
    SIF_CHECK(near_eq(camera.world_to_screen({0.f, 0.f}), math::Point2(200.f, 400.f)));
}

SIF_TEST(camera_zoom_magnifies_around_the_center) {
    rnd::Camera camera({800.f, 800.f}, rnd::AspectPolicy::Fit);
    SIF_CHECK(near_eq(camera.scale().x, 400.f));

    camera.set_zoom(2.f);
    SIF_CHECK(near_eq(camera.scale().x, 800.f));
    // The centre itself never moves under zoom.
    SIF_CHECK(near_eq(camera.world_to_screen({0.f, 0.f}), math::Point2(400.f, 400.f)));
}

SIF_TEST(camera_rejects_a_non_positive_zoom_instead_of_collapsing) {
    rnd::Camera camera({800.f, 800.f});
    camera.set_zoom(2.f);

    camera.set_zoom(0.f);
    SIF_CHECK(near_eq(camera.zoom(), 2.f));

    camera.set_zoom(-1.f);
    SIF_CHECK(near_eq(camera.zoom(), 2.f));
}

SIF_TEST(camera_y_axis_up_mirrors_only_the_vertical_axis) {
    rnd::Camera camera({800.f, 800.f}, rnd::AspectPolicy::Fit);
    camera.set_y_axis_up(true);

    // World +y is now "up", so it maps to a smaller screen y.
    SIF_CHECK(near_eq(camera.world_to_screen({-1.f, 1.f}), math::Point2(0.f, 0.f)));
    SIF_CHECK(near_eq(camera.world_to_screen({-1.f, -1.f}), math::Point2(0.f, 800.f)));
    // Round-tripping still holds with the axis flipped.
    SIF_CHECK(near_eq(camera.screen_to_world(camera.world_to_screen({0.3f, 0.7f})),
                      math::Point2(0.3f, 0.7f)));
}

SIF_TEST(camera_projects_a_world_rect_to_a_normalised_screen_rect) {
    const rnd::Camera camera({800.f, 800.f}, rnd::AspectPolicy::Fit);

    // A quarter-world tile in the bottom-right quadrant.
    const intrnl::Rect world{0.f, 0.f, 0.5f, 0.5f};
    SIF_CHECK(near_eq(camera.world_to_screen(world), intrnl::Rect(400.f, 400.f, 200.f, 200.f)));

    // With the y axis flipped the same rect still comes back with a
    // top-left origin and positive extents.
    rnd::Camera flipped({800.f, 800.f}, rnd::AspectPolicy::Fit);
    flipped.set_y_axis_up(true);
    const intrnl::Rect projected = flipped.world_to_screen(world);
    SIF_CHECK(projected.width > 0.f && projected.height > 0.f);
    SIF_CHECK(near_eq(projected, intrnl::Rect(400.f, 200.f, 200.f, 200.f)));
}

SIF_TEST(camera_world_sizes_convert_to_pixel_sizes) {
    const rnd::Camera camera({800.f, 800.f}, rnd::AspectPolicy::Fit);
    // 2 world units span the full 800px, so 0.25 units is 100px.
    SIF_CHECK(near_eq(camera.world_to_screen_size({0.25f, 0.25f}).x, 100.f));
    SIF_CHECK(near_eq(camera.world_to_screen_size({-0.25f, -0.25f}).y, 100.f));
}

SIF_TEST(camera_reports_the_visible_world_region_for_culling) {
    rnd::Camera camera({800.f, 800.f}, rnd::AspectPolicy::Fit);

    SIF_CHECK(near_eq(camera.visible_world_bounds(), intrnl::Rect(-1.f, -1.f, 2.f, 2.f)));
    SIF_CHECK(camera.is_visible({-0.1f, -0.1f, 0.2f, 0.2f}));
    SIF_CHECK(!camera.is_visible({5.f, 5.f, 1.f, 1.f}));

    // Zooming in halves what is visible, so a tile near the edge drops out.
    camera.set_zoom(4.f);
    SIF_CHECK(near_eq(camera.visible_world_bounds(), intrnl::Rect(-0.25f, -0.25f, 0.5f, 0.5f)));
    SIF_CHECK(!camera.is_visible({0.8f, 0.8f, 0.1f, 0.1f}));
}

SIF_TEST(camera_can_project_into_a_sub_viewport) {
    rnd::Camera camera({800.f, 600.f}, rnd::AspectPolicy::Stretch);
    // e.g. a board panel next to a HUD column.
    camera.set_viewport({200.f, 0.f, 400.f, 400.f});

    SIF_CHECK(near_eq(camera.world_to_screen({0.f, 0.f}), math::Point2(400.f, 200.f)));
    SIF_CHECK(near_eq(camera.world_to_screen({-1.f, -1.f}), math::Point2(200.f, 0.f)));
    SIF_CHECK(near_eq(camera.world_to_screen({1.f, 1.f}), math::Point2(600.f, 400.f)));

    // A viewport with no area is refused rather than dividing by zero.
    camera.set_viewport({0.f, 0.f, 0.f, 100.f});
    SIF_CHECK(near_eq(camera.viewport(), intrnl::Rect(200.f, 0.f, 400.f, 400.f)));
}

SIF_TEST(camera_without_a_target_degrades_instead_of_dividing_by_zero) {
    const rnd::Camera camera;

    SIF_CHECK(near_eq(camera.scale().x, 0.f));
    // screen_to_world has no inverse here, so it answers with the centre.
    SIF_CHECK(near_eq(camera.screen_to_world({123.f, 456.f}), math::Point2(0.f, 0.f)));
    SIF_CHECK(!camera.is_visible({-1.f, -1.f, 2.f, 2.f}));
}

// ---------------------------------------------------------------------
// Atomic JSON writing (io::write_json_file)
//
// The registry and every *.asset.json go through this one function, and
// each of these cases used to be a real failure: an aborted tool, a
// silently truncated source file, or a "success" that wrote nothing.
// ---------------------------------------------------------------------

SIF_TEST(write_json_file_creates_missing_parent_directories) {
    const std::filesystem::path dir = std::filesystem::temp_directory_path() / "sif_json_test" / "deep";
    const std::filesystem::path file = dir / "out.json";
    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "sif_json_test");

    nlohmann::json j = nlohmann::json::array();
    j.push_back(42);
    io::write_json_file(file, j);

    SIF_CHECK(std::filesystem::exists(file));

    std::ifstream in(file);
    nlohmann::json read;
    in >> read;
    SIF_CHECK(read.is_array() && read[0] == 42);

    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "sif_json_test");
}

SIF_TEST(write_json_file_leaves_no_temp_file_behind) {
    const std::filesystem::path file = std::filesystem::temp_directory_path() / "sif_json_tmp.json";
    std::filesystem::path tmp = file;
    tmp += ".tmp";
    std::filesystem::remove(file);
    std::filesystem::remove(tmp);

    io::write_json_file(file, nlohmann::json::object());

    SIF_CHECK(std::filesystem::exists(file));
    SIF_CHECK(!std::filesystem::exists(tmp));

    std::filesystem::remove(file);
}

SIF_TEST(write_json_file_replaces_an_existing_file_atomically) {
    const std::filesystem::path file = std::filesystem::temp_directory_path() / "sif_json_replace.json";

    nlohmann::json first = nlohmann::json::object();
    first["v"] = 1;
    io::write_json_file(file, first);

    nlohmann::json second = nlohmann::json::object();
    second["v"] = 2;
    io::write_json_file(file, second);

    std::ifstream in(file);
    nlohmann::json read;
    in >> read;
    SIF_CHECK(read["v"] == 2);

    std::filesystem::remove(file);
}

SIF_TEST(write_json_file_reports_an_unusable_path) {
    bool threw = false;
    try {
        // A path whose "directory" is an existing regular file cannot
        // be created; this must be an error, not a silent no-op.
        const std::filesystem::path blocker = std::filesystem::temp_directory_path() / "sif_json_blocker";
        { std::ofstream(blocker) << "x"; }
        io::write_json_file(blocker / "inside" / "out.json", nlohmann::json::object());
        std::filesystem::remove(blocker);
    } catch (const std::exception&) {
        threw = true;
        std::filesystem::remove(std::filesystem::temp_directory_path() / "sif_json_blocker");
    }
    SIF_CHECK(threw);
}
