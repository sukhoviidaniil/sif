/***************************************************************
 * Project:       Render_Engine (demos)
 * File:          DemoSupport.h
 *
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_DEMOSUPPORT_H
#define RENDER_ENGINE_DEMOSUPPORT_H

#include <memory>
#include <string>

#include "sif/asset/AssetHandle.h"
#include "sif/asset/internal/Sound.h"
#include "sif/internal/GUID.h"
#include "sif/layout_engine/elements/UIElement.h"

namespace app::demo {
    /**
     * @brief Where the demo data lives, resolved once at start-up.
     *
     * Set by main() from argv/the working directory, so the demos never
     * hard-code a relative path of their own (the old app baked
     * "../data/" into main and nothing else could be run from anywhere
     * else).
     */
    struct Paths {
        static void set_data_dir(const std::string& dir);
        [[nodiscard]] static const std::string& data_dir();

        /// @brief data_dir() + "bin/scenes/" + file
        [[nodiscard]] static std::string scene(const std::string& file);
    };

    /**
     * @brief Tokenizes, parses and builds a serialized *.ui.xml scene.
     *
     * The whole authoring pipeline in one call: Tokenizer -> Parser ->
     * UIFactory, exactly as a game would do it.
     */
    [[nodiscard]] std::unique_ptr<sif::ui::UIElement> load_scene(const std::string& scene_file);

    /**
     * @brief Looks up an asset GUID by its human-readable asset name.
     *
     * Demos refer to sounds by name (as the authoring *.ui.xml files do
     * for images) instead of pasting numeric GUIDs into C++.
     */
    [[nodiscard]] sif::intrnl::GUID guid_of(const std::string& asset_name);

    /**
     * @brief Requests a Sound asset by name and returns a handle to it.
     */
    [[nodiscard]] sif::asset::AssetHandle<sif::asset::Sound> request_sound(const std::string& asset_name);

    /**
     * @brief Sets the text of a named ui::Text element, if it exists.
     *
     * Silently does nothing when the element is missing, so a demo does
     * not crash because a label was renamed in the scene file.
     */
    void set_label(sif::ui::UIElement& root, const std::string& element_name, const std::string& value);
}

#endif //RENDER_ENGINE_DEMOSUPPORT_H
