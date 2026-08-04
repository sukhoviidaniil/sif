/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-08-04
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"

#include <string>

#include "sif/asset/internal/AssetImporter.h"

using namespace sif;

SIF_TEST(registry_names_must_end_with_the_registry_extension) {
    SIF_CHECK(asset::is_registry_filename("registry.rgst.json"));
    SIF_CHECK(asset::is_registry_filename("data/bin/registry.rgst.json"));
    SIF_CHECK(asset::is_registry_filename("/abs/path/level_1.rgst.json"));
    SIF_CHECK(asset::is_registry_filename("a.rgst.json"));
}

SIF_TEST(registry_names_reject_anything_else) {
    SIF_CHECK(!asset::is_registry_filename("registry.json"));
    SIF_CHECK(!asset::is_registry_filename("registry.rgst"));
    SIF_CHECK(!asset::is_registry_filename("registry"));
    SIF_CHECK(!asset::is_registry_filename("data/bin/registry.RGST.JSON")); // case matters
    SIF_CHECK(!asset::is_registry_filename("font.asset.json"));
    SIF_CHECK(!asset::is_registry_filename(""));
}

SIF_TEST(registry_name_without_a_stem_is_rejected) {
    // ".rgst.json" carries no <name>, so two registries in one folder
    // could not be told apart.
    SIF_CHECK(!asset::is_registry_filename(".rgst.json"));
    SIF_CHECK(!asset::is_registry_filename("data/bin/.rgst.json"));
}

SIF_TEST(registry_name_suggestions_replace_the_tail_instead_of_stacking_it) {
    SIF_CHECK(asset::suggested_registry_filename("registry.json") == "registry.rgst.json");
    SIF_CHECK(asset::suggested_registry_filename("registry.rgst") == "registry.rgst.json");
    SIF_CHECK(asset::suggested_registry_filename("registry") == "registry.rgst.json");
    SIF_CHECK(asset::suggested_registry_filename("registry.rgst.json") == "registry.rgst.json");
}

SIF_TEST(registry_name_suggestions_keep_the_directory) {
    SIF_CHECK(asset::suggested_registry_filename("data/bin/registry.json")
              == "data/bin/registry.rgst.json");
    SIF_CHECK(asset::suggested_registry_filename("out/level_1")
              == "out/level_1.rgst.json");
}

SIF_TEST(registry_name_suggestions_never_produce_an_empty_stem) {
    SIF_CHECK(asset::suggested_registry_filename(".rgst.json") == "registry.rgst.json");
    SIF_CHECK(asset::suggested_registry_filename("data/.json") == "data/registry.rgst.json");
}

SIF_TEST(suggested_registry_filenames_are_themselves_valid) {
    // Whatever we tell the user to type must pass the check, otherwise
    // the diagnostic sends them in a circle.
    for (const std::string input : {"registry.json", "registry", "registry.rgst",
                                    ".rgst.json", "a/b/c.txt", "data/bin/reg.rgst.json"}) {
        SIF_CHECK(asset::is_registry_filename(asset::suggested_registry_filename(input)));
    }
}
