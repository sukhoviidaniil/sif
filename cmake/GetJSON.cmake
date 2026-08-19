# ---------------------------------------------------------------------------
# Provides nlohmann/json's single header at external/json/json.hpp.
#
# Unlike SFML, this is not optional for any part of sif: the asset system
# (AssetImporter, AssetRegistry, every *.asset.json descriptor) is built on
# nlohmann::json, and that is true even of the pure engine with every
# SFML-consuming target turned off. So unlike cmake/GetSFML.cmake, this file
# is always included, and unlike SFML there is no "skip gracefully" branch -
# a build that cannot get this file cannot build sif at all, and says so.
#
# Three sources, in order:
#
#   1. external/json/json.hpp already present - vendored by hand, dropped in
#      by a packaging step, or left over from an earlier configure. Used
#      as-is, nothing is downloaded, no network access happens.
#   2. -DSIF_JSON_SOURCE=<path> - an explicit local copy to use instead.
#   3. downloaded from nlohmann/json's own GitHub release assets. That
#      specific file - not the repository's single_include/ path, a release
#      asset - is what the project publishes for exactly this integration
#      method (https://github.com/nlohmann/json#integration: "you can also
#      just grab the file from ... the release page"), so this is the
#      header's actual origin, not a mirror of a mirror.
#
# The downloaded file is checked against a pinned SHA-256 before anything
# is allowed to compile against it: a header this central silently
# corrupted or swapped in transit would not fail loudly, it would fail
# *strangely*, in whatever translation unit happened to trip over the
# damaged bytes first.
# ---------------------------------------------------------------------------

set(SIF_JSON_VERSION "3.11.3" CACHE STRING
        "nlohmann/json release to fetch when external/json/json.hpp is not already present")

# Updated whenever SIF_JSON_VERSION is. Verified directly against the
# release asset at that version - see the comment on the download step
# below for how to re-derive it if the version above ever changes.
set(SIF_JSON_SHA256 "9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6")

set(SIF_JSON_SOURCE "" CACHE FILEPATH
        "Local json.hpp to use instead of downloading one (skips the hash check - it is your copy)")

set(SIF_JSON_DIR "${PROJECT_SOURCE_DIR}/external/json")
set(SIF_JSON_HEADER "${SIF_JSON_DIR}/json.hpp")

if(EXISTS "${SIF_JSON_HEADER}")
    message(STATUS "nlohmann/json: using existing ${SIF_JSON_HEADER}")
elseif(SIF_JSON_SOURCE)
    if(NOT EXISTS "${SIF_JSON_SOURCE}")
        message(FATAL_ERROR "nlohmann/json: SIF_JSON_SOURCE='${SIF_JSON_SOURCE}' does not exist")
    endif()
    message(STATUS "nlohmann/json: copying the provided ${SIF_JSON_SOURCE}")
    file(MAKE_DIRECTORY "${SIF_JSON_DIR}")
    file(COPY_FILE "${SIF_JSON_SOURCE}" "${SIF_JSON_HEADER}")
else()
    message(STATUS "nlohmann/json: fetching v${SIF_JSON_VERSION} (first configure only)")

    # EXPECTED_HASH also makes file(DOWNLOAD) verify-then-fail atomically:
    # a mismatched or truncated download does not get left at the
    # destination path for a later, unguarded build step to pick up.
    file(DOWNLOAD
            "https://github.com/nlohmann/json/releases/download/v${SIF_JSON_VERSION}/json.hpp"
            "${SIF_JSON_HEADER}"
            SHOW_PROGRESS
            STATUS SIF_JSON_DOWNLOAD_STATUS
            EXPECTED_HASH SHA256=${SIF_JSON_SHA256}
            TLS_VERIFY ON
    )

    list(GET SIF_JSON_DOWNLOAD_STATUS 0 SIF_JSON_DOWNLOAD_CODE)
    if(NOT SIF_JSON_DOWNLOAD_CODE EQUAL 0)
        list(GET SIF_JSON_DOWNLOAD_STATUS 1 SIF_JSON_DOWNLOAD_MESSAGE)
        file(REMOVE "${SIF_JSON_HEADER}") # don't leave a partial/mismatched file behind
        message(FATAL_ERROR
                "nlohmann/json: failed to fetch v${SIF_JSON_VERSION}: ${SIF_JSON_DOWNLOAD_MESSAGE}\n"
                "Either provide network access on first configure, place json.hpp yourself at\n"
                "    ${SIF_JSON_HEADER}\n"
                "or point at a local copy with -DSIF_JSON_SOURCE=<path-to-json.hpp>.\n"
                "(If SIF_JSON_VERSION was changed and this is a hash mismatch: re-derive\n"
                "SIF_JSON_SHA256 in this file from the new release asset - see the comment\n"
                "above it.)")
    endif()
endif()

set(JSON_DIR "${SIF_JSON_DIR}")
