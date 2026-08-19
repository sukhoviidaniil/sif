# ---------------------------------------------------------------------------
# sif_sfml - the reference backend, promoted to a real target.
#
# Renderer, audio player, event collector and one asset loader per asset
# type (sfml/), plus the CPU-only backend used for headless asset
# verification (headless/), plus the Abstract Factory that switches between
# them by ast::RB_Type (Graphics_Factory.*).
#
# This used to be something every consumer had to glob and compile for
# itself - Bomberman's own CMakeLists.txt did exactly that, with a comment
# explaining it was a stand-in for "a proper sif_sfml target". This is that
# target: linking sif_sfml is now one line,
#     target_link_libraries(your_view PUBLIC sif_sfml)
# and SFML comes along transitively - nothing else to glob, configure, or
# hand SFML targets to.
#
# The three pieces are one target rather than three, because sif_headless
# and sif_sfml both link the *library* SFML (for sf::Image / sf::SoundBuffer
# decoding) even though only sif_sfml touches a window or an audio device,
# and Graphics_Factory has to see both to dispatch between them by RB_Type -
# splitting them apart would just move that coupling into the build graph
# instead of removing it.
# ---------------------------------------------------------------------------

# SFML is located exactly once, at the root CMakeLists.txt - see the
# comment there. This target simply checks that the search succeeded.
if(NOT TARGET sfml-graphics)
    # Unlike sif/tools' sprite packer, this is not an optional nicety: a
    # consumer link-references sif_sfml directly, and a silently-missing
    # target would surface three files away as "target sif_sfml not found"
    # instead of here, where the actual cause is one line above.
    message(FATAL_ERROR
            "sif: SIF_BUILD_SFML_BACKEND is ON but no SFML 2.6.x could be found or "
            "fetched. Install it (Ubuntu: sudo apt install libsfml-dev), point at one "
            "with -DSFML_DIR=..., or turn this backend off with "
            "-DSIF_BUILD_SFML_BACKEND=OFF if you do not need it.")
endif()

file(GLOB SIF_SFML_SRC CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/sfml/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/headless/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Graphics_Factory.cpp"
)

add_library(sif_sfml STATIC ${SIF_SFML_SRC})

target_include_directories(sif_sfml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# PUBLIC: these sources #include <SFML/...> directly, and so does anything
# that calls into sif::sfml types (e.g. constructing an SFML_Renderer), so
# the include directories and link dependency both have to reach whoever
# links sif_sfml - not just this target itself.
target_link_libraries(sif_sfml
        PUBLIC
        sif_lib
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
)
