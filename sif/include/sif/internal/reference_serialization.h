/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-17
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_REFERENCE_SERIALIZATION_H
#define RENDER_ENGINE_REFERENCE_SERIALIZATION_H
#include <filesystem>
#include <iostream>
#include <string>

#include "sif/asset/internal/AssetImporter.h"
#include "sif/diagnostics/Logger.h"
#include "sif/layout_engine/Token.h"
#include "sif/layout_engine/Tokenizer.h"

#include <stdexcept>

namespace sif {
    /**
     * @brief Rewrites authoring scenes (asset_name="...") into runtime
     * scenes (guid="...").
     *
     * @return How many files failed. Failures are logged per file and
     * do not stop the run, but they must not be reported as success
     * either - the caller turns a non-zero result into a non-zero exit
     * code, so a broken asset reference cannot slip through CI.
     */
    inline size_t reference_serialization(const std::string& dir, const std::string& serialized_dir) {
        namespace fs = std::filesystem;

        if (!fs::exists(dir)) {
            throw std::runtime_error("Scenes directory does not exist: " + dir);
        }
        if (!fs::is_directory(dir)) {
            throw std::runtime_error("Scenes directory is not a directory: " + dir);
        }

        size_t failed = 0;
        size_t processed = 0;

        for (auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!entry.is_regular_file())
                continue;

            const std::string& path = entry.path().string();

            if (!sif::ui::Tokenizer::is_ui_xml_file(path))
                continue;
            try {
                std::vector<sif::ui::Token> tokens = sif::ui::Tokenizer::tokenize(path);

                fs::path relative_path = fs::relative(entry.path(), dir);

                fs::path target_path = fs::path(serialized_dir) / relative_path;

                fs::create_directories(target_path.parent_path());

                for (sif::ui::Token& token : tokens) {
                    auto it = token.attributes.find("asset_name");
                    if (it != token.attributes.end()) {
                        std::string asset_name = it->second;
                        asset::AssetDesc desc = sif::asset::AssetImporter::instance().get(asset_name);
                        intrnl::GUID guid = desc.meta.guid;

                        token.attributes.erase(it);
                        token.attributes.emplace("guid", guid.string());
                    }
                }

                sif::ui::Tokenizer::save_tokens(target_path.string(), tokens);
                ++processed;
            } catch (const std::exception& e) {
                ++failed;
                const std::string err = "Reference Serialization - Failed to process file '" + path + "': " + e.what();
                LOG(err);
                std::cerr << err << std::endl;
            }
        }

        LOG("Reference Serialization - processed " + std::to_string(processed) + " scene(s), " +
            std::to_string(failed) + " failed");

        return failed;
    }
} // namespace sif

#endif // RENDER_ENGINE_REFERENCE_SERIALIZATION_H