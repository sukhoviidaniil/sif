/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-15
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#include <iostream>

#include "sif/diagnostics/Logger.h"
#include "sif/asset/internal/AssetImporter.h"

int main(int argc, char* argv[]) {
    std::string graphics_dir;
    std::string registry_file;

    if (argc == 1) {
        graphics_dir = "data/graphics/";
        registry_file = "regitry";
    } else if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
          << " <graphics_dir> <registry_file>" << std::endl;
        return 1;
    } else {
        graphics_dir = argv[1];
        registry_file = argv[2];
    }

    sif::asset::AssetImporter::instance().load_from_path(graphics_dir);
    sif::asset::AssetImporter::instance().save_in_file(registry_file);
    return 0;
}
