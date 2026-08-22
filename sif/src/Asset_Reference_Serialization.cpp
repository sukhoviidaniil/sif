/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-15
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include <exception>
#include <iostream>

#include "sif/asset/internal/AssetImporter.h"
#include "sif/internal/reference_serialization.h"

int main(const int argc, char* argv[]) {

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <registry_file> <scenes_dir> <serialized_scenes_dir>\n"
                  << "  registry_file           produced by Asset_GUID_Assignment\n"
                  << "  scenes_dir              authoring scenes (asset_name=\"...\")\n"
                  << "  serialized_scenes_dir   output scenes (guid=\"...\"), created if missing\n";
        return 1;
    }

    const std::string registry_file = argv[1];
    const std::string scenes_dir = argv[2];
    const std::string serialized_scenes_dir = argv[3];

    try {
        sif::asset::AssetImporter::instance().load_from_file(registry_file);
        if (const size_t failed = sif::reference_serialization(scenes_dir, serialized_scenes_dir); failed > 0) {
            std::cerr << "Asset_Reference_Serialization: " << failed << " scene(s) could not be serialized\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Asset_Reference_Serialization failed: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Asset_Reference_Serialization failed: unknown error\n";
        return 1;
    }

    std::cout << "Serialized scenes written to " << serialized_scenes_dir << '\n';
    return 0;
}
