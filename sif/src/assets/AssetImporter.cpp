/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-16
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/asset/internal/AssetImporter.h"
#include "sif/asset/internal/data/AssetDataLoader.h"
#include "sif/asset/AssetRegistry.h"
#include "sif/internal/from_JSON.h"
#include "sif/diagnostics/Logger.h"
#include "json.hpp"

#include <algorithm>
#include <vector>
#include <fstream>

namespace sif::asset {

    bool is_registry_filename(const std::string &filepath) {
        const std::string name = std::filesystem::path(filepath).filename().string();

        // The suffix alone is not enough: ".rgst.json" has no <name>
        // part and would make two different registries in one directory
        // indistinguishable.
        if (name.size() <= registry_extension.size()) {
            return false;
        }
        return name.compare(name.size() - registry_extension.size(),
                            registry_extension.size(),
                            registry_extension) == 0;
    }

    std::string suggested_registry_filename(const std::string &filepath) {
        const std::filesystem::path path(filepath);
        std::string name = path.filename().string();

        // Strip the tail we are about to rewrite, so "registry.json"
        // does not become "registry.json.rgst.json".
        // >= and not >: a filename that consists of nothing but the
        // suffix (".json", ".rgst.json") must strip down to an empty
        // stem so the fallback below can name it, instead of being
        // treated as a hidden file and getting a second suffix glued on.
        const auto strip = [&name](const std::string_view suffix) {
            if (name.size() >= suffix.size() &&
                name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0) {
                name.erase(name.size() - suffix.size());
            }
        };
        strip(".json");
        strip(".rgst");

        if (name.empty()) {
            name = "registry";
        }
        name += registry_extension;

        const std::filesystem::path parent = path.parent_path();
        return parent.empty() ? name : (parent / name).generic_string();
    }


    AssetImporter& AssetImporter::instance() {
        static AssetImporter inst;
        return inst;
    }

    AssetImporter::AssetImporter() = default;

    void AssetImporter::load_from_path(const std::string& dirpath) {
        std::lock_guard lock(mtx_);

        std::filesystem::path root(dirpath);
        // Silently returning here produced the worst possible outcome:
        // an empty importer, a registry file containing "[]", and an
        // exit code of 0 - i.e. a tool that reported success while
        // scanning nothing. A wrong path is a user error and has to say
        // so.
        if (!std::filesystem::exists(root)) {
            throw std::runtime_error("Asset directory does not exist: " + dirpath);
        }
        if (!std::filesystem::is_directory(root)) {
            throw std::runtime_error("Asset path is not a directory: " + dirpath);
        }

        // Note: previously imported descriptors are deliberately kept.
        // Running load_from_file() and then load_from_path() is the
        // normal update flow - it is what keeps already-assigned GUIDs
        // stable when a new asset appears.
        size_t found = 0;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            if (entry.path().extension() != ".json" || !entry.path().filename().string().ends_with(".asset.json")) {
                continue; // read only *.asset.json
            }


            auto data_node = data::AssetDataLoader::load_from_file(entry.path());
            if (data_node == nullptr) {
                continue;
            }

            auto rel_path = entry.path().lexically_relative(root);

            // generic_string(), not string(): on Windows the latter
            // emits backslashes, which then get baked into the registry
            // file and cannot be opened again on Linux/macOS. '/' is
            // accepted by every platform we build on.
            asset::AssetDesc desc(
                rel_path.generic_string(),
                data_node->meta
                );

            add(desc);

            data::AssetDataLoader::save_Meta(entry.path(), desc.meta);
            ++found;
        }

        LOG("AssetImporter: scanned " + dirpath + ", found " +
            std::to_string(found) + " asset descriptor(s)");
    }

    void AssetImporter::save_in_file(const std::string &filepath) {
        std::shared_lock lock(mtx_);

        // Sorted by asset name rather than emitted in unordered_map
        // order, so regenerating the registry without changing anything
        // produces an identical file instead of a shuffled diff.
        std::vector<const AssetDesc*> ordered;
        ordered.reserve(by_guid_.size());
        for (const auto& [guid, desc] : by_guid_) {
            ordered.push_back(&desc);
        }
        std::sort(ordered.begin(), ordered.end(), [](const AssetDesc* a, const AssetDesc* b) {
            if (a->meta.asset_name != b->meta.asset_name) {
                return a->meta.asset_name < b->meta.asset_name;
            }
            return a->meta.guid.string() < b->meta.guid.string();
        });

        nlohmann::json j = nlohmann::json::array();
        for (const AssetDesc* desc : ordered) {
            j.push_back(nlohmann::json(*desc)); // relies on to_json(AssetDesc)
        }

        if (ordered.empty()) {
            // Not fatal - an empty project is legal - but it is almost
            // always a wrong input path, and silence here is what makes
            // that take an hour to find.
            LOG("AssetImporter: writing an EMPTY registry to " + filepath +
                " - no assets have been imported");
        }

        if (!is_registry_filename(filepath)) {
            LOG("AssetImporter: '" + filepath + "' does not use the required " +
                std::string(registry_extension) + " registry naming");
        }

        // Creates missing directories, writes to a temp file and renames
        // it into place, and verifies the stream afterwards.
        io::write_json_file(filepath, j);

        LOG("AssetImporter: wrote " + std::to_string(ordered.size()) +
            " asset(s) to " + filepath);
    }

    void AssetImporter::load_from_file(const std::string& filepath) {
        std::lock_guard lock(mtx_);

        std::filesystem::path path(filepath);
        // Returning quietly here meant a missing registry produced an
        // importer that knows nothing, and every later lookup failed
        // with a message about the *asset* rather than about the
        // registry that was never read.
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Registry file does not exist: " + filepath);
        }

        // A warning rather than an error: an existing project must keep
        // loading. It is still worth saying, because a registry that is
        // not named like one is usually a sign that something else was
        // passed by mistake.
        if (!is_registry_filename(filepath)) {
            LOG("AssetImporter: loading '" + filepath + "' which is not named <name>" +
                std::string(registry_extension));
        }

        std::ifstream in(path, std::ios::binary);
        if (!in) {
            throw std::runtime_error("Failed to open file: " + filepath);
        }

        nlohmann::json j;
        in >> j;

        if (!j.is_array()) {
            throw std::runtime_error("Invalid asset file format: expected JSON array");
        }

        by_guid_.clear();
        by_asset_name_.clear();
        guids_.clear();

        for (const auto& jd : j) {
            auto desc = io::get_checked<AssetDesc>(jd);
            // Tolerate registries written by an older (Windows) build
            // that stored backslash-separated paths.
            std::replace(desc.conf_path.begin(), desc.conf_path.end(), '\\', '/');
            add(desc);
        }
    }

    void AssetImporter::add(AssetDesc& desc) {
        intrnl::GUID& g = desc.meta.guid;
        std::string asset_name = desc.meta.asset_name;
        auto it= by_asset_name_.find(asset_name);
        if (it != by_asset_name_.end()) {
            intrnl::GUID& existing = it->second.meta.guid;
            if (g != existing) {
                LOG(
                    "Asset_name - " + asset_name +
                    " | GUID - " + g.string() +
                    " collides with GUID - " + existing.string()
                    );
            }
            return;
        }

        if (guids_.contains(g)) {
            g = intrnl::generate_guid(guids_);
        }

        guids_.insert(g);
        by_guid_.emplace(g, desc);
        by_asset_name_.emplace(asset_name, desc);
    }


    void AssetImporter::load_in_registry() {
        std::shared_lock lock(mtx_);
        for (const auto& [guid, desc] : by_guid_) {
            AssetRegistry::instance().add_AssetRecord(desc);
        }
    }

    AssetDesc AssetImporter::get(const std::string &id) const {
        std::shared_lock lock(mtx_);
        const auto it = by_asset_name_.find(id);
        if (it == by_asset_name_.end()) {
            throw std::runtime_error("Asset not found by name");
        }
        return it->second;
    }

}

