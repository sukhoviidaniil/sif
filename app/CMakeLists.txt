# ---------------------------------------------------------------------------
# Demo application: the SFML backend plus the demo scenarios.
#
# This is the only target that is allowed to know SFML exists. sif_lib
# itself links nothing graphical, which is what lets the engine and its
# tests build on a machine without SFML installed.
# ---------------------------------------------------------------------------

# Prefer a vendored SFML if one is present, but fall back to the system
# package (which is what the reference platform provides). The previous
# version *overwrote* CMAKE_PREFIX_PATH with a hard-coded path, so a
# checkout without external/SFML-2.6.1 could not configure at all.
set(SFML_RELATIVE_DIR "external/SFML-2.6.1")
if(WIN32)
    set(SFML_RELATIVE_DIR "${SFML_RELATIVE_DIR}/windows")
elseif(UNIX)
    set(SFML_RELATIVE_DIR "${SFML_RELATIVE_DIR}/linux")
endif()
set(SFML_ROOT "${CMAKE_SOURCE_DIR}/${SFML_RELATIVE_DIR}")
if(EXISTS "${SFML_ROOT}/lib/cmake/SFML")
    list(PREPEND CMAKE_PREFIX_PATH "${SFML_ROOT}/lib/cmake/SFML")
    message(STATUS "Using vendored SFML from ${SFML_ROOT}")
endif()

find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

set(SFML_SIF_DEMO_APP "sif_demo")

set(SFML_SIF_DEMO_SRC
        main.cpp
        App.cpp
        Graphics_Factory.cpp
        demos/DemoSupport.cpp
        demos/Demos.cpp
        demos/HeadlessCheck.cpp
        headless/HeadlessBackend.cpp
        sfml/SFMLAssetLoaders.cpp
        sfml/SFML_AudioPlayer.cpp
        sfml/SFML_Event_Collector.cpp
        sfml/SFML_Renderer.cpp
)

add_executable(${SFML_SIF_DEMO_APP} ${SFML_SIF_DEMO_SRC})

target_include_directories(${SFML_SIF_DEMO_APP} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${SFML_SIF_DEMO_APP}
        PRIVATE
        sif_lib
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
)
