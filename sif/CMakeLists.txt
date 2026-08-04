# ---------------------------------------------------------------------------
# sif library build configuration.
#
# The engine is split into one static library per concern/folder, rather
# than one monolithic sif_lib, so dependency direction is explicit and
# enforced by the linker (not just "please don't include that header"):
#
#   sif_core    (internal/, io/)   - GUID/RecordID/Rect/Color/Size/JSON
#                                    helpers/etc; no dependency on any
#                                    other sif_* library.
#   sif_math    (math/)            - Vector/Matrix/Vector2/Point2/Graph;
#                                    depends only on sif_core (Rect).
#   sif_diagnostics (diagnostics/) - Logger/LogScope plus the include-cycle
#                                    analysis tool (CycleFinder etc, which
#                                    is built on math::DirectedGraph).
#   sif_event   (event/)           - Event_Bus/Observer/event definitions;
#                                    depends only on sif_core.
#   sif_asset   (assets/)          - "Model": asset loading/registry/queue.
#   sif_render  (render/)          - "View": render-node visitor tree.
#   sif_layout_engine (layout_engine/)
#                                  - "Controller": HTML-like XML -> UI
#                                    element tree -> render items; ties
#                                    together asset (Text/Sprite use
#                                    AssetHandle) and render (UIElement
#                                    emits RenderNode items).
#
# sif_lib remains as a single INTERFACE target aggregating all of the
# above, so existing consumers (app/, the Asset_* tools below) keep
# linking against one name.
# ---------------------------------------------------------------------------

set(SIF_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ========== sif_core ==========
file(GLOB_RECURSE SIF_CORE_SRC
    "src/internal/*.cpp"
)
add_library(sif_core STATIC ${SIF_CORE_SRC})
target_include_directories(sif_core PUBLIC ${SIF_INCLUDE_DIR} ${JSON_DIR})

# ========== sif_math ==========
file(GLOB_RECURSE SIF_MATH_SRC
    "src/math/*.cpp"
)
add_library(sif_math STATIC ${SIF_MATH_SRC})
target_link_libraries(sif_math PUBLIC sif_core)

# ========== sif_diagnostics ==========
file(GLOB_RECURSE SIF_DIAGNOSTICS_SRC
    "src/diagnostics/*.cpp"
)
add_library(sif_diagnostics STATIC ${SIF_DIAGNOSTICS_SRC})
target_link_libraries(sif_diagnostics PUBLIC sif_math)

# ========== sif_event ==========
file(GLOB_RECURSE SIF_EVENT_SRC
    "src/event/*.cpp"
)
add_library(sif_event STATIC ${SIF_EVENT_SRC})
target_link_libraries(sif_event PUBLIC sif_core)

# ========== sif_asset (Model) ==========
file(GLOB_RECURSE SIF_ASSET_SRC
    "src/assets/*.cpp"
)
add_library(sif_asset STATIC ${SIF_ASSET_SRC})
target_link_libraries(sif_asset PUBLIC sif_core sif_math sif_diagnostics)

# ========== sif_audio (audio output interface) ==========
file(GLOB_RECURSE SIF_AUDIO_SRC
    "src/audio/*.cpp"
)
add_library(sif_audio STATIC ${SIF_AUDIO_SRC})
target_link_libraries(sif_audio PUBLIC sif_core sif_asset)

# ========== sif_render (View) ==========
file(GLOB_RECURSE SIF_RENDER_SRC
    "src/render/*.cpp"
)
add_library(sif_render STATIC ${SIF_RENDER_SRC})
target_link_libraries(sif_render PUBLIC sif_core sif_math sif_event sif_asset)

# ========== sif_layout_engine (Controller) ==========
file(GLOB_RECURSE SIF_LAYOUT_ENGINE_SRC
    "src/layout_engine/*.cpp"
)
add_library(sif_layout_engine STATIC ${SIF_LAYOUT_ENGINE_SRC})
target_link_libraries(sif_layout_engine PUBLIC sif_core sif_math sif_diagnostics sif_asset sif_render)

# ========== sif_lib (aggregate, for existing/simple consumers) ==========
add_library(sif_lib INTERFACE)
target_link_libraries(sif_lib INTERFACE
    sif_core
    sif_math
    sif_diagnostics
    sif_event
    sif_asset
    sif_audio
    sif_render
    sif_layout_engine
)

# configuration Asset_GUID_Assignment
set(ASSET_GUID_ASSIGNMENT "Asset_GUID_Assignment")
add_executable(${ASSET_GUID_ASSIGNMENT} ${CMAKE_CURRENT_SOURCE_DIR}/src/Asset_GUID_Assignment.cpp)
target_link_libraries(${ASSET_GUID_ASSIGNMENT} PRIVATE sif_lib)

# configuration Asset_Reference_Serialization
set(ASSET_REFERENCE_SERIALIZATION "Asset_Reference_Serialization")
add_executable(${ASSET_REFERENCE_SERIALIZATION} ${CMAKE_CURRENT_SOURCE_DIR}/src/Asset_Reference_Serialization.cpp)
target_link_libraries(${ASSET_REFERENCE_SERIALIZATION} PRIVATE sif_lib)

# ========== Tests ==========
# See sif/test/CMakeLists.txt: a separate, optional target so consumers
# that only want the engine itself are not forced to build tests.
option(SIF_BUILD_TESTS "Build the sif test suite" ON)
if(SIF_BUILD_TESTS)
    add_subdirectory(test)
endif()
