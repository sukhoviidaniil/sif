# ---------------------------------------------------------------------------
# sif_demo - the reference demo application.
#
# Everything SFML-specific now lives in sif_sfml (sif/backends/); this
# target compiles only the demo's own sources and links that one library -
# renderer, audio player, event collector, every asset loader and SFML
# itself all come along transitively.
# ---------------------------------------------------------------------------

# SFML is located exactly once, at the root CMakeLists.txt, whenever
# SIF_BUILD_DEMO_APP (this option) is enabled - see the comment there.
if(NOT TARGET sfml-graphics)
    message(STATUS "sif: SFML not found, skipping sif_demo (pass -DSIF_FETCH_SFML=ON or install it to build the demo)")
    return()
endif()

if(NOT TARGET sif_sfml)
    message(FATAL_ERROR
            "sif: SIF_BUILD_DEMO_APP is ON but sif_sfml is not - the demo needs the "
            "reference backend. Enable it with -DSIF_BUILD_SFML_BACKEND=ON.")
endif()

set(SFML_SIF_DEMO_APP "sif_demo")

add_executable(${SFML_SIF_DEMO_APP}
        main.cpp
        App.cpp
        demos/DemoSupport.cpp
        demos/Demos.cpp
        demos/HeadlessCheck.cpp
)

target_include_directories(${SFML_SIF_DEMO_APP} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${SFML_SIF_DEMO_APP} PRIVATE sif_lib sif_sfml)
