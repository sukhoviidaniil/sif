/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UI_SCENECOMPOSER_H
#define RENDER_ENGINE_UI_SCENECOMPOSER_H

#include <memory>
#include <string>
#include <vector>

#include "sif/layout_engine/Node.h"
#include "sif/layout_engine/elements/UIElement.h"

namespace sif::ui {
/**
 * @brief Resolves <Include> and <Style>/class in a parsed scene.
 *
 * A pass over the Node tree between Parser and UIFactory, so no element
 * ever learns that styles or includes exist - by the time UIFactory
 * builds the tree, every attribute an element sees is its own.
 *
 *  - <Include src="panel.ui.xml"/> is replaced by the root of that file
 *    (its path is relative to the including file). Any other attribute
 *    on the <Include> overrides the included root's attribute of the
 *    same name, so one panel can be included twice under two names or
 *    sizes. Includes nest; a cycle is an error.
 *  - <Style name="title" color="..." fontSize="26"/> declares a named
 *    set of attributes, anywhere in the document (included files
 *    included); Style nodes are removed from the tree.
 *  - class="title wide" applies those sets in order: a later class
 *    overrides an earlier one, and the element's own attributes override
 *    every class.
 *
 * An unknown class is reported like an unknown attribute: a warning by
 * default, an error in strict mode (see UIFactory::set_strict, whose
 * setting compose() follows).
 */
class SceneComposer {
public:
    /**
     * @brief Resolves includes and styles in root, in place.
     *
     * @param root Parsed scene (from Parser::parse).
     * @param source_path File the scene was read from; includes are
     * resolved relative to its directory, and every node is stamped with
     * the file it came from (Node::source).
     * @param warnings Receives warnings (unknown classes), if non-null.
     * @throws SceneError on a missing/invalid include, an include cycle,
     * a nameless or duplicate Style, or (strict mode) an unknown class.
     */
    static void compose(Node& root, const std::string& source_path, std::vector<std::string>* warnings = nullptr);
};

/**
 * @brief The whole scene pipeline in one call.
 *
 * Tokenizer -> Parser -> SceneComposer -> UIFactory. The recommended way
 * to load a *.ui.xml scene: building a parsed tree directly with
 * UIFactory still works, but skips includes and styles.
 */
class SceneLoader {
public:
    /**
     * @param path Scene file.
     * @param warnings Receives every warning of the load (unknown
     * classes, unknown attributes), in order, if non-null.
     * @throws std::runtime_error / SceneError if the file cannot be read,
     * parsed, composed or built.
     */
    static std::unique_ptr<UIElement> load(const std::string& path, std::vector<std::string>* warnings = nullptr);
};
} // namespace sif::ui

#endif // RENDER_ENGINE_UI_SCENECOMPOSER_H
