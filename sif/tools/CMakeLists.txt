# ---------------------------------------------------------------------------
# sif asset tools that need image I/O.
#
# The engine links no graphics library, but *packing sprite strips* is
# fundamentally an image operation, so this one tool borrows SFML's CPU-side
# sf::Image. No window and no OpenGL context is involved, which is why it runs
# on a build agent.
#
# If SFML is not available the tool is simply skipped rather than failing the
# configure: an engine-only build (the CI job that proves sif compiles without
# SFML) must keep working, and a project that has no per-frame art never needs
# the packer.
# ---------------------------------------------------------------------------

# A consumer that already located SFML - the usual case, because it needs it
# for its own rendering - shares that copy instead of running a second search
# that might land on a different version.
if(NOT TARGET sfml-graphics)
    find_package(SFML 2.6 COMPONENTS graphics system QUIET)
endif()

if(NOT TARGET sfml-graphics)
    message(STATUS "sif: SFML not found, skipping sif_sprite_packer (the engine itself does not need it)")
    return()
endif()

add_executable(sif_sprite_packer "${CMAKE_CURRENT_SOURCE_DIR}/Sprite_Packer.cpp")
target_link_libraries(sif_sprite_packer PRIVATE sif_lib sfml-graphics sfml-system)
