/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-16
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_ASSETIMPORTER_H
#define RENDER_ENGINE_ASSETIMPORTER_H

#include <shared_mutex>
#include <unordered_map>

#include <string_view>

#include "AssetDesc.h"
#include "AssetRecord.h"
#include "sif/internal/GUID.h"

namespace sif::asset {

    /**
     * @brief The mandatory suffix of a registry file: <name>.rgst.json
     *
     * Registries are generated artefacts that sit next to hand-authored
     * *.asset.json descriptors and ordinary *.json data. A dedicated,
     * unambiguous suffix is what lets a build script, a .gitignore rule
     * or a human tell at a glance which files are generated and safe to
     * delete - and it stops a mistyped argument from quietly
     * overwriting an asset descriptor with a registry.
     */
    inline constexpr std::string_view registry_extension = ".rgst.json";

    /**
     * @brief Checks that a path names a registry file.
     *
     * @return true only if the *filename* is <name>.rgst.json with a
     * non-empty <name>. Directories are not inspected, so any location
     * is acceptable.
     */
    [[nodiscard]] bool is_registry_filename(const std::string& filepath);

    /**
     * @brief Proposes a conforming name for a path that fails
     * is_registry_filename.
     *
     * Used to make the diagnostics actionable ("did you mean ...?")
     * rather than merely correct. Any trailing .json / .rgst is
     * replaced, never stacked, so registry.json and registry.rgst both
     * suggest registry.rgst.json.
     */
    [[nodiscard]] std::string suggested_registry_filename(const std::string& filepath);

    class AssetImporter {
    public:
        static AssetImporter& instance();

        void load_from_path(const std::string& dirpath);
        void save_in_file(const std::string& filepath);
        void load_from_file(const std::string& filepath);

        void load_in_registry();

        AssetDesc get(const std::string& id) const;

    private:
        AssetImporter();
        void add(AssetDesc& desc);
        mutable std::shared_mutex mtx_;

        std::unordered_set<intrnl::GUID, intrnl::GUIDHash> guids_;

        std::unordered_map<intrnl::GUID, AssetDesc, intrnl::GUIDHash> by_guid_;

        std::unordered_map<std::string, AssetDesc> by_asset_name_;
    };
} // namespace sif::asset

#endif // RENDER_ENGINE_ASSETIMPORTER_H