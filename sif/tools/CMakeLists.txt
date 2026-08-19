# ---------------------------------------------------------------------------
# sif asset tools that need image I/O.
#
# The engine links no graphics library, but *packing sprite strips* is
# fundamentally an image operation, so this one tool borrows SFML's CPU-side
# sf::Image. No window and no OpenGL context is involved, which is why it
# runs on a build agent.
#
# sif_sprite_packer stays optional and quiet about it: an engine-only build
# (the CI job that proves sif compiles without SFML) must keep working, and
# a project with no per-frame art never needs the packer. Unlike sif_sfml,
# nothing links against this target, so there is no confusing failure to
# prevent three files away - a skipped tool is simply a tool nobody sees.
# ---------------------------------------------------------------------------

# SFML is located exactly once, at the root CMakeLists.txt, if
# SIF_BUILD_TOOLS, SIF_BUILD_SFML_BACKEND or SIF_BUILD_DEMO_APP is enabled -
# see the comment there for why every subdirectory that needs SFML has to
# rely on that single root-level search rather than repeating it locally.
if(NOT TARGET sfml-graphics)
    message(STATUS "sif: SFML not found, skipping sif_sprite_packer (the engine itself does not need it)")
    return()
endif()

add_executable(sif_sprite_packer "${CMAKE_CURRENT_SOURCE_DIR}/Sprite_Packer.cpp")
target_link_libraries(sif_sprite_packer PRIVATE sif_lib sfml-graphics sfml-system)
