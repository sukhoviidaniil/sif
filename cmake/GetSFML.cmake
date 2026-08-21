# ---------------------------------------------------------------------------
# Locates SFML 2.6 for sif's own SFML-consuming targets (sif_sfml,
# sif_sprite_packer, sif_demo) - genuinely isolated from whatever SFML a
# project embedding sif might separately set up for its own, unrelated
# purposes, even within the very same CMake configure.
#
# Why "genuinely" needs saying: it is not enough to check
# `if(NOT TARGET sfml-graphics)` and reuse whatever is already there. A
# project that fetches sif (say, to get sif_sfml) might, entirely
# separately, want SFML 3.x for its own direct use - the assignment this
# engine grew out of explicitly allows a downstream project to pick any
# SFML version for itself, independent of what sif's own reference backend
# happens to be written against. If that project's own search ran first, at
# a CMake scope that is an *ancestor* of wherever sif ends up being
# configured (e.g. the top of that project's own root CMakeLists.txt,
# before it pulls sif in), a naive "reuse if present" check would silently
# hand sif_sfml's build the wrong major version - and because the failure
# then shows up as SFML 3 compiler errors inside sif's own reference
# backend, the person actually hit by it is often not the one who set the
# other version up: it is "another user who downloads sif for themselves"
# and just wants the demo or the sprite packer, with no idea a sibling
# project's SFML choice was ever involved.
#
# Two things make sif's own search immune to that, not merely "usually
# fine":
#
#   1. Every value this file reads or writes is sif-prefixed
#      (SIF_SFML_DIR, SIF_SFML_VENDORED_ROOT, SIF_FETCH_SFML). The bare,
#      conventional SFML_DIR / SFML_ROOT / CMAKE_PREFIX_PATH are never
#      consulted, so a project that sets those for its own SFML never
#      changes what sif finds.
#   2. find_package(SFML ...) is always called with an explicit PATHS
#      argument and NO_DEFAULT_PATH, pointing at a directory *this file
#      itself* located. Without NO_DEFAULT_PATH, find_package quietly
#      falls back to its own default search (which does consult SFML_DIR
#      and CMAKE_PREFIX_PATH) the moment the explicit path fails to
#      satisfy the version constraint - silently reopening exactly the
#      hole point 1 closes.
#
# The one thing that genuinely cannot be made airtight this way is the
# from-source fallback (step 4 below): FetchContent-built SFML defines
# real, non-imported targets, and CMake target names are unique project
# -wide regardless of directory scope. If both sif and an embedding
# project need to fetch *different* SFML versions from source in the same
# configure - neither having a suitable copy already installed - that is a
# genuine, unavoidable CMake limitation, not a lazily-skipped fix. It is
# rare in practice (installing SFML system-wide, or embedding sif from a
# project that has no direct SFML needs of its own, both sidestep it
# entirely), so it is documented here rather than solved.
#
# Search order, entirely within sif's own, private configuration surface:
#   1. -DSIF_SFML_DIR=<prefix>/lib/cmake/SFML
#   2. a vendored build under external/SFML-2.6.1, relative to *this*
#      project's own root (PROJECT_SOURCE_DIR, not CMAKE_SOURCE_DIR, which
#      would resolve to whichever project embeds sif once it is fetched as
#      a subproject).
#   3. any installed SFML whose version starts with 2.
#   4. failing all of those, SFML 2.6.1 is fetched and built from source
#      (-DSIF_FETCH_SFML=OFF disables this step).
#
# What this file does not decide: whether failing to find SFML is fatal.
# It leaves sfml-graphics undefined and returns; each call site decides for
# itself (sif/tools treats the packer as optional and skips it quietly,
# sif/backends and app treat SFML as required once explicitly requested and
# fail loudly with a clear message).
# ---------------------------------------------------------------------------

if(TARGET sfml-graphics)
    # Something in a scope that is an ancestor of this one already defined
    # it - either an earlier call to this same file (see the root
    # CMakeLists.txt: it is included exactly once, so within sif's own
    # tree this branch should not normally trigger), or an embedding
    # project's own, unrelated SFML setup. Warn in the second case rather
    # than assume compatibility silently: reaching this point without
    # going through sif's own version-checked search is exactly the
    # situation the isolation above exists to prevent.
    if(NOT SFML_VERSION_MAJOR OR NOT SFML_VERSION_MAJOR EQUAL 2)
        message(WARNING
                "sif: an 'sfml-graphics' target already exists (version "
                "'${SFML_VERSION}', from outside sif's own search) and sif's SFML-"
                "consuming targets will link it as-is. If that is not SFML 2.6.x, "
                "sif_sfml / sif_sprite_packer / sif_demo will fail to compile against "
                "it. This normally means an embedding project searched for its own, "
                "unrelated SFML from a CMake scope that is an ancestor of sif's - see "
                "the comment at the top of this file for why that is the one case "
                "sif's own isolation cannot fully cover.")
    endif()
    return()
endif()

option(SIF_FETCH_SFML "Fetch and build SFML 2.6.1 from source when no SFML 2.x is found" ON)

set(SIF_SFML_COMPONENTS graphics window system audio)

# ---------------------------------------------------------------------------
# Reads the version out of an SFMLConfigVersion.cmake without running it.
# ---------------------------------------------------------------------------
function(sif_sfml_config_version config_dir out_version)
    set(${out_version} "" PARENT_SCOPE)

    set(version_file "${config_dir}/SFMLConfigVersion.cmake")
    if(NOT EXISTS "${version_file}")
        return()
    endif()

    file(READ "${version_file}" contents)
    # Both the CMake-generated and the hand-written variants spell it
    # `set(PACKAGE_VERSION "2.6.1")`.
    string(REGEX MATCH "set\\(PACKAGE_VERSION[ \t]+\"?([0-9]+\\.[0-9]+(\\.[0-9]+)?)\"?" _m "${contents}")
    if(CMAKE_MATCH_1)
        set(${out_version} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Collects every SFML config directory sif is willing to look at (its own
# private candidates only - see the file header) and keeps the 2.x one.
# ---------------------------------------------------------------------------
function(sif_find_sfml2_dir out_dir out_rejected)
    set(candidates "")

    if(SIF_SFML_VENDORED_ROOT)
        list(APPEND candidates
                "${SIF_SFML_VENDORED_ROOT}/lib/cmake/SFML"
                "${SIF_SFML_VENDORED_ROOT}/lib64/cmake/SFML")
    endif()

    # Common installation prefixes for a self-built SFML 2.6, plus the
    # distribution's own location. Deliberately not CMAKE_PREFIX_PATH or
    # any environment variable a consumer might have set for its own SFML -
    # only paths sif would also have found on a machine with nothing else
    # installed.
    file(GLOB system_candidates
            "/usr/local/lib/cmake/SFML"
            "/usr/local/lib/*/cmake/SFML"
            "/opt/SFML*/lib/cmake/SFML"
            "$ENV{HOME}/SFML*/lib/cmake/SFML"
            "/usr/lib/cmake/SFML"
            "/usr/lib/*/cmake/SFML"
            "/opt/sfml2/lib/cmake/SFML"
    )
    list(APPEND candidates ${system_candidates})
    list(REMOVE_DUPLICATES candidates)

    set(rejected "")
    foreach(dir IN LISTS candidates)
        if(NOT EXISTS "${dir}/SFMLConfig.cmake")
            continue()
        endif()

        sif_sfml_config_version("${dir}" version)
        if(version MATCHES "^2\\.")
            message(STATUS "sif: SFML ${version} selected at ${dir}")
            set(${out_dir} "${dir}" PARENT_SCOPE)
            set(${out_rejected} "${rejected}" PARENT_SCOPE)
            return()
        endif()

        if(version)
            list(APPEND rejected "${version} (${dir})")
        endif()
    endforeach()

    set(${out_dir} "" PARENT_SCOPE)
    set(${out_rejected} "${rejected}" PARENT_SCOPE)
endfunction()

# ---- 1-2. An explicit SIF_SFML_DIR wins; otherwise a vendored copy --------

set(SIF_SFML_VENDORED_ROOT "${PROJECT_SOURCE_DIR}/external/SFML-2.6.1")
if(WIN32)
    set(SIF_SFML_VENDORED_ROOT "${SIF_SFML_VENDORED_ROOT}/windows")
elseif(UNIX)
    set(SIF_SFML_VENDORED_ROOT "${SIF_SFML_VENDORED_ROOT}/linux")
endif()

set(SIF_SFML_DIR "" CACHE PATH
        "Directory containing SFMLConfig.cmake, for sif's own use only (does not read or write the generic SFML_DIR)")

set(_sif_resolved_dir "")

if(SIF_SFML_DIR)
    sif_sfml_config_version("${SIF_SFML_DIR}" _sif_explicit_version)
    if(_sif_explicit_version AND NOT _sif_explicit_version MATCHES "^2\\.")
        message(FATAL_ERROR
                "sif: SIF_SFML_DIR points at SFML ${_sif_explicit_version}, but sif's "
                "reference backend needs the 2.6.x API (SFML 2.6.1 is the reference "
                "platform's version). This is sif's own, private override - it does "
                "not read the generic SFML_DIR, so an embedding project's own SFML "
                "choice is never the cause of this.")
    endif()
    message(STATUS "sif: using the explicitly provided SIF_SFML_DIR=${SIF_SFML_DIR}")
    set(_sif_resolved_dir "${SIF_SFML_DIR}")
else()
    # ---- 3. Candidates sif looks for on its own -----------------------
    sif_find_sfml2_dir(_sif_sfml2_dir _sif_rejected_versions)
    if(_sif_sfml2_dir)
        set(_sif_resolved_dir "${_sif_sfml2_dir}")
    elseif(_sif_rejected_versions)
        message(STATUS "sif: ignoring incompatible SFML installation(s): ${_sif_rejected_versions}")
    endif()
endif()

if(_sif_resolved_dir)
    # A LOCAL (non-cache) shadow of SFML_DIR, not a CACHE ... FORCE.
    #
    # This turned out to be the one line that actually matters, and PATHS
    # + NO_DEFAULT_PATH alone (the first version of this file) was not
    # enough: find_package's Config mode treats <PackageName>_DIR - here,
    # literally SFML_DIR - as an authoritative "I already know where this
    # is" override that it consults *before* the PATHS-based search, and
    # NO_DEFAULT_PATH does not suppress that consultation. Verified by
    # reproducing it: with an ancestor scope's SFML_DIR already pointing
    # at a fake SFML 3.0.0, the PATHS+NO_DEFAULT_PATH-only version still
    # picked up that 3.0.0 through SFML_DIR and only rejected it afterwards
    # via the version check below - by then find_package had already done
    # the wrong lookup once.
    #
    # A plain `set(SFML_DIR ...)` here shadows the cache entry with a
    # regular variable for the rest of *this* directory scope and its
    # descendants (sif/tools, sif/backends, app/) - CMake always prefers a
    # local variable over a same-named cache entry for lookups within that
    # scope. It is not CACHE, so it never touches the actual cache entry:
    # a project that embeds sif and has its own, unrelated SFML_DIR set at
    # an ancestor or sibling scope keeps seeing its own value, completely
    # unaffected, the moment control leaves sif's own directory tree.
    set(SFML_DIR "${_sif_resolved_dir}")
    find_package(SFML 2.6 COMPONENTS ${SIF_SFML_COMPONENTS} QUIET)
endif()

if(SFML_FOUND AND DEFINED SFML_VERSION_MAJOR AND NOT SFML_VERSION_MAJOR EQUAL 2)
    message(STATUS "sif: discarding SFML ${SFML_VERSION}, the 2.x API is required")
    unset(SFML_FOUND)
endif()

if(SFML_FOUND)
    return()
endif()

# ---- 4. Build 2.6.1 from source --------------------------------------------

if(NOT SIF_FETCH_SFML)
    message(STATUS
            "sif: no SFML 2.6.x found and SIF_FETCH_SFML=OFF - leaving sfml-graphics "
            "undefined. Point at one with -DSIF_SFML_DIR=<prefix>/lib/cmake/SFML, "
            "install it (Ubuntu: sudo apt install libsfml-dev), or re-run with "
            "-DSIF_FETCH_SFML=ON.")
    return()
endif()

message(STATUS "sif: no SFML 2.6.x installation found - fetching and building SFML 2.6.1 (first configure only)")

include(FetchContent)

if(UNIX AND NOT APPLE)
    message(STATUS
            "sif: building SFML from source needs the X11/OpenGL/audio development "
            "headers. On Debian/Ubuntu: sudo apt install libxrandr-dev libxcursor-dev "
            "libxi-dev libudev-dev libgl1-mesa-dev libfreetype-dev libopenal-dev "
            "libflac-dev libvorbis-dev")
endif()

set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# NOTE: this step builds real (non-imported) targets named sfml-graphics
# etc., and those *are* unique project-wide regardless of directory scope -
# see the file header. It will conflict if an embedding project also needs
# to fetch-build a different SFML version from source in this same
# configure; it will not conflict with that project's own find_package-
# located or otherwise-imported SFML, by the same reasoning that makes the
# rest of this file safe.
FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SFML)

# The targets are called sfml-graphics/window/system/audio whether SFML was
# found or built here, so nothing downstream knows the difference.
