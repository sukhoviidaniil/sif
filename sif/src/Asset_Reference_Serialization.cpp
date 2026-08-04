/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-15
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/internal/reference_serialization.h"
#include "sif/asset/internal/AssetImporter.h"

int main(int argc, char* argv[]) {

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <registry_file> <scenes_dir> <serialized_scenes_dir>" << std::endl;
        return 1;
    }

    const std::string registry_file = argv[1];

    const std::string scenes_dir = argv[2];
    const std::string serialized_scenes_dir = argv[3];

    sif::asset::AssetImporter::instance().load_from_file(registry_file);
    sif::reference_serialization(scenes_dir, serialized_scenes_dir);

    return 0;
}