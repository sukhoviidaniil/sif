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
#include <string>

#include "sif/asset/internal/AssetImporter.h"
#include "sif/diagnostics/Logger.h"

namespace {
    /**
     * @brief Warns when the output path is not named <name>.rgst.json.
     *
     * Deliberately a warning and not a hard failure: refusing to write
     * would break any existing pipeline that already points somewhere
     * else, and the tool's job is still perfectly well defined. But the
     * naming matters - a registry is a generated file that lives among
     * hand-authored *.asset.json descriptors, and the distinct suffix
     * is what keeps build scripts, .gitignore rules and humans from
     * confusing the two (or from overwriting a descriptor with a
     * registry because two arguments were swapped).
     *
     * @return true if the name conforms.
     */
    bool warn_if_not_registry_name(const std::string& registry_file) {
        if (sif::asset::is_registry_filename(registry_file)) {
            return true;
        }

        const std::string suggestion = sif::asset::suggested_registry_filename(registry_file);

        std::cerr << "Warning: '" << registry_file << "' is not a registry name.\n"
                  << "         Registry files must be named <name>" << sif::asset::registry_extension
                  << " - did you mean '" << suggestion << "'?\n"
                  << "         Writing to the given path anyway.\n";

        LOG("Asset_GUID_Assignment: non-conforming registry name '" + registry_file + "', expected <name>" +
            std::string(sif::asset::registry_extension));

        return false;
    }
} // namespace

/**
 * @brief Scans a data directory, assigns/collects asset GUIDs and writes
 * the registry file the runtime loads.
 *
 * Re-runnable: existing GUIDs in the *.asset.json files are kept, only
 * colliding or missing ones are generated, and the registry is written
 * atomically.
 */
int main(const int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <asset_dir> <registry_file>\n"
                  << "  asset_dir      directory scanned recursively for *.asset.json\n"
                  << "  registry_file  registry to write, named <name>" << sif::asset::registry_extension
                  << " (parent directories are created)\n";
        return 1;
    }

    const std::string asset_dir = argv[1];
    const std::string registry_file = argv[2];

    warn_if_not_registry_name(registry_file);

    // Without this, a missing output directory or a mistyped input path
    // reached std::terminate and the only trace was "Aborted".
    try {
        sif::asset::AssetImporter& importer = sif::asset::AssetImporter::instance();
        importer.load_from_path(asset_dir);
        importer.save_in_file(registry_file);
    } catch (const std::exception& e) {
        std::cerr << "Asset_GUID_Assignment failed: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Asset_GUID_Assignment failed: unknown error\n";
        return 1;
    }

    std::cout << "Registry written to " << registry_file << '\n';
    return 0;
}
