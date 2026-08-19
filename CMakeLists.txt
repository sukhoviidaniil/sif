cmake_minimum_required(VERSION 3.20)
# Specify the name of the project
project(sif LANGUAGES CXX)

# We specify the c++ standard
set(CMAKE_CXX_STANDARD 20)
# If the compiler can't support it, it's an error
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# We force the compiler to disable all extensions and strictly follow the standard
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
    message("-- Debugmod on")
endif()

# Every target sif defines - the engine libraries, the asset tools, the
# reference backend, the demo, the tests - lands in one place inside sif's
# own build tree, the way SFML's own build groups sfml-system, sfml-window,
# sfml-graphics, sfml-audio and sfml-network into one lib/ folder instead of
# scattering them across a build directory per source subdirectory.
#
# PROJECT_BINARY_DIR, not CMAKE_BINARY_DIR: the latter is always the
# outermost build tree, which - once sif is fetched as a subproject - is the
# *consumer's* build directory. PROJECT_BINARY_DIR tracks the most recent
# project() call, so from here down it stays sif's own corner of the build
# tree regardless of who is building it, and this setting therefore never
# reaches out and rearranges a consumer's own targets.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# PROJECT_SOURCE_DIR, not CMAKE_SOURCE_DIR: the latter is always the
# outermost project's root, which - once sif is fetched as a subproject -
# is the *consumer's* source tree, not sif's own. That mismatch used to
# make sif look for nlohmann/json inside whichever project embedded it,
# rather than in its own external/json/ - and that directory used to have
# to be populated by hand (or by a consumer's own workaround) before this
# file could do anything with it. cmake/GetJSON.cmake now gets it itself,
# fetching the header from its own upstream release the first time nothing
# is found there, so a fresh clone with nothing but this repository builds
# on its own.
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GetJSON.cmake")

# ---------------------------------------------------------------------------
# What gets built, and how eagerly each of them reaches for SFML.
#
# The engine itself (sif_lib, added unconditionally below) never links SFML
# and never searches for it - that is the whole point of the logic/view
# split the assignment this engine grew out of asks for. Everything below
# is opt-in, and each option's SFML acquisition is genuinely lazy: with all
# three off, cmake/GetSFML.cmake is never even included, so a plain
# `cmake -S . -B build` on a machine with no SFML at all configures and
# builds the engine and nothing tries to touch a graphics library.
# ---------------------------------------------------------------------------

# The asset tools (Asset_GUID_Assignment, Asset_Reference_Serialization,
# sif_sprite_packer) are genuinely useful to any consumer's build pipeline,
# so they default on; only sif_sprite_packer needs SFML, and it degrades to
# "skipped, with a message" rather than failing the configure when SFML is
# unavailable - see sif/tools/CMakeLists.txt.
option(SIF_BUILD_TOOLS "Build the sif asset tools" ON)

# The reference backend (sif_sfml: renderer, audio player, event collector,
# every asset loader, plus the CPU-only headless variant) is what a consumer
# like Bomberman actually links against, so it also defaults on. Unlike the
# tools, this one is not optional once requested - see
# sif/backends/CMakeLists.txt for why a silent skip here would be worse than
# a clear failure.
option(SIF_BUILD_SFML_BACKEND "Build the sif_sfml reference backend" ON)

# The demo application is a windowed program with its own asset registry
# and scenes - not something a library consumer needs, and not something
# that should turn a routine `cmake -S . -B build` into a full SFML build.
# It is therefore off by default and, like the tools, skips itself quietly
# if SFML cannot be found or fetched rather than failing the whole configure
# - building the demo is a request for a nice-to-have reference program, not
# a precondition for anything else in this repository.
option(SIF_BUILD_DEMO_APP "Build the SFML demo application" OFF)

# Included exactly once, here, rather than separately from sif/tools,
# sif/backends and app/. SFML's own Config-mode package files declare their
# targets as IMPORTED but not GLOBAL, and a non-GLOBAL imported target is
# only visible to the directory scope that created it *and that scope's own
# descendants* - not to sibling subdirectories. sif/tools and sif/backends
# are siblings in the directory-scope tree (one reached via sif/CMakeLists.txt,
# the other added directly from here), so calling the module separately from
# each looked idempotent but silently repeated the whole search every time.
# Doing it once at the root - the common ancestor of every subdirectory that
# might need it - is what actually makes the "on demand, once" promise true.
#
# cmake/GetSFML.cmake is deliberately self-contained: it never reads or
# writes the generic SFML_DIR/SFML_ROOT (only its own SIF_SFML_DIR), and
# every find_package call it makes is pinned with NO_DEFAULT_PATH to a
# directory it located itself. That is what keeps sif's own SFML choice
# genuinely independent of whatever SFML version a project embedding sif
# (Bomberman, for instance) might separately set up for its own direct use -
# including a different major version - without either one needing to know
# the other exists.
if(SIF_BUILD_TOOLS OR SIF_BUILD_SFML_BACKEND OR SIF_BUILD_DEMO_APP)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/GetSFML.cmake")
endif()

add_subdirectory(sif)

if(SIF_BUILD_SFML_BACKEND)
    add_subdirectory(sif/backends)
endif()

if(SIF_BUILD_DEMO_APP)
    add_subdirectory(app)
endif()
