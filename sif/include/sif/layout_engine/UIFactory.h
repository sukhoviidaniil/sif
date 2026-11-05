/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UIFACTORY_H
#define RENDER_ENGINE_UIFACTORY_H

#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "sif/layout_engine/Node.h"
#include "sif/layout_engine/elements/UIElement.h"

namespace sif::ui {

/**
 * @brief An error in a scene, already prefixed with where it is.
 *
 * Its own type so that UIFactory adds the "line L, column C" prefix
 * exactly once: an error thrown while building a deeply nested element
 * passes through every ancestor's build untouched instead of collecting
 * one prefix per level.
 */
class SceneError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class UIFactory {
public:
    using Builder = std::function<std::unique_ptr<UIElement>(const Node&)>;

    static UIFactory& instance();

    /**
     * @brief Registers a tag without an attribute schema.
     *
     * Kept for tags registered from outside the engine: such a tag
     * accepts any attribute, exactly as every tag did before schemas
     * existed.
     */
    void register_tag(std::string tag, Builder fn);

    /**
     * @brief Registers a tag together with the attributes it understands.
     *
     * The schema lives next to the registration (see the constructor),
     * so adding an attribute to a make_* handler and to its tag's list
     * is one change in one file. Attributes common to every element
     * (name, id, visible, layer, ... - see add_common_attributes) are
     * always accepted and need not be listed.
     */
    void register_tag(std::string tag, Builder fn, std::initializer_list<const char*> attributes);

    /**
     * @brief Builds the element tree for a parsed scene.
     *
     * Every node is checked against its tag's schema. An unknown
     * attribute - usually a typo such as fill_colour - is reported with
     * its line and column and, when a known attribute is close, a
     * suggestion. In the default mode the report is a warning (logged
     * and kept in warnings()), so a scene that worked before keeps
     * loading; in strict mode it is a SceneError. Any other error while
     * building an element is rethrown as a SceneError carrying the
     * element's position.
     *
     * @throws SceneError on an unknown tag, an invalid attribute value,
     * or (strict mode) an unknown attribute.
     */
    std::unique_ptr<UIElement> build(const Node& n);

    /**
     * @brief Turns unknown attributes into errors instead of warnings.
     *
     * Off by default so existing scenes keep loading; meant for tests,
     * CI and scene tooling, where a typo should fail loudly.
     */
    void set_strict(bool strict);
    [[nodiscard]] bool strict() const;

    /// @brief Warnings produced by the most recent build(), in document order.
    [[nodiscard]] const std::vector<std::string>& warnings() const;

private:
    struct TagEntry {
        Builder make;
        /// nullopt: no schema, every attribute accepted.
        std::optional<std::unordered_set<std::string>> attributes;
    };

    UIFactory();

    std::unique_ptr<UIElement> build_node(const Node& n);
    void check_attributes(const Node& n, const TagEntry& entry);

    std::unordered_map<std::string, TagEntry> map_;
    bool strict_ = false;
    std::vector<std::string> warnings_;
};
} // namespace sif::ui

#endif // RENDER_ENGINE_UIFACTORY_H
