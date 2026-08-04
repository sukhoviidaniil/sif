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

set(JSON_DIR "${CMAKE_SOURCE_DIR}/external/json")
message(STATUS "nlohmann/json include dir: ${JSON_DIR}")

# The demo application needs SFML; the engine does not. Turning this
# off gives a build that compiles and tests sif on a machine with no
# graphics stack at all (which is how CI verifies the engine).
option(SIF_BUILD_DEMO_APP "Build the SFML demo application" ON)

add_subdirectory(sif)

# add_subdirectory(app) used to appear twice - once guarded, once not -
# which made configuring with the option enabled a hard CMake error.
if(SIF_BUILD_DEMO_APP)
    add_subdirectory(app)
endif()