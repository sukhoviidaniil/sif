/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-13
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/layout_engine/UIFactory.h"

#include <functional>

#include "sif/layout_engine/internal/add_attribute.h"
#include "sif/layout_engine/internal/make_element.h"

namespace sif::ui {
    using UIFactoryFn = std::function<std::unique_ptr<UIElement>(const Node&)>;

    UIFactory& UIFactory::instance() {
        static UIFactory f;
        return f;
    }

    void UIFactory::register_tag(std::string tag, UIFactoryFn fn)  {
        map_[std::move(tag)] = std::move(fn);
    }

    std::unique_ptr<UIElement> UIFactory::build(const Node &n)  {
        const auto it = map_.find(n.tag);

        if (it == map_.end()) {
            throw std::runtime_error("Unknown tag: " + n.tag);
        }

        auto el = it->second(n);

        // "name"/"id" are common to every element (used by
        // UIElement::find_by_name / find_by_id and by ui::Menu), so
        // they are applied once here rather than in every make_* tag
        // handler.
        add_attribute_name(n, el.get());
        add_attribute_id(n, "Node <" + n.tag + ">", el.get());

        for (auto& c : n.children) {
            el->add(build(*c));
        }

        return el;
    }

    // Singleton pattern: exactly one UIFactory maps tag names to
    // element constructors for the whole program, so every part of the
    // engine that parses UI XML shares the same set of registered tags.
    UIFactory::UIFactory() {
        register_tag("Element", make_Element);
        register_tag("VBox", make_VBox);
        register_tag("HBox", make_HBox);
        register_tag("Text", make_Text);
        register_tag("Rectangle", make_Rectangle);
        register_tag("Sprite", make_Sprite);
        register_tag("Animation", make_Animation);
        register_tag("Button", make_Button);
        register_tag("Menu", make_Menu);
    }
}
