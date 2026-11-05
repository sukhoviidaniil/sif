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

#include <algorithm>
#include <functional>

#include "sif/diagnostics/Logger.h"
#include "sif/layout_engine/internal/add_attribute.h"
#include "sif/layout_engine/internal/make_element.h"

namespace sif::ui {
namespace {
/**
 * @brief Attributes every element understands (see add_common_attributes).
 */
const std::unordered_set<std::string>& common_attributes() {
    static const std::unordered_set<std::string> common = {
        "name",   "id",      "visible", "z_index", "layer",     "hit_testable", "clip",
        "margin", "h_align", "v_align", "offset",  "focusable", "world_rect"};
    return common;
}

/// @brief "line L, column C: " for a node read from a file, "" otherwise.
std::string where(const Node& n) {
    if (n.line == 0) {
        return n.source.empty() ? std::string{} : n.source + ": ";
    }
    const std::string position = "line " + std::to_string(n.line) + ", column " + std::to_string(n.column) + ": ";
    return n.source.empty() ? position : n.source + ", " + position;
}

/// @brief Classic edit distance, used only to suggest a fix for a typo.
std::size_t edit_distance(const std::string& a, const std::string& b) {
    std::vector<std::size_t> row(b.size() + 1);
    for (std::size_t j = 0; j <= b.size(); ++j) {
        row[j] = j;
    }
    for (std::size_t i = 1; i <= a.size(); ++i) {
        std::size_t diagonal = row[0];
        row[0] = i;
        for (std::size_t j = 1; j <= b.size(); ++j) {
            const std::size_t above = row[j];
            row[j] = std::min({row[j] + 1, row[j - 1] + 1, diagonal + (a[i - 1] == b[j - 1] ? 0u : 1u)});
            diagonal = above;
        }
    }
    return row[b.size()];
}

/// @brief The known attribute closest to name, if it is close enough to be a typo.
std::string closest(const std::string& name, const std::unordered_set<std::string>& known) {
    std::string best;
    std::size_t best_distance = 3; // more than two edits away is not a typo
    for (const std::unordered_set<std::string>* set : {&known, &common_attributes()}) {
        for (const std::string& candidate : *set) {
            const std::size_t d = edit_distance(name, candidate);
            if (d < best_distance || (d == best_distance && !best.empty() && candidate < best)) {
                best_distance = d;
                best = candidate;
            }
        }
    }
    return best;
}
} // namespace

UIFactory& UIFactory::instance() {
    static UIFactory f;
    return f;
}

void UIFactory::register_tag(std::string tag, Builder fn) { map_[std::move(tag)] = TagEntry{std::move(fn), {}}; }

void UIFactory::register_tag(std::string tag, Builder fn, const std::initializer_list<const char*> attributes) {
    std::unordered_set<std::string> known;
    for (const char* a : attributes) {
        known.emplace(a);
    }
    map_[std::move(tag)] = TagEntry{std::move(fn), std::move(known)};
}

std::unique_ptr<UIElement> UIFactory::build(const Node& n) {
    warnings_.clear();
    return build_node(n);
}

void UIFactory::set_strict(const bool strict) { strict_ = strict; }

bool UIFactory::strict() const { return strict_; }

const std::vector<std::string>& UIFactory::warnings() const { return warnings_; }

void UIFactory::check_attributes(const Node& n, const TagEntry& entry) {
    if (!entry.attributes.has_value()) {
        return;
    }

    // Sorted, so the report (and therefore warnings()) does not depend
    // on unordered_map iteration order.
    std::vector<std::string> unknown;
    for (const auto& [attribute, value] : n.attributes) {
        if (!entry.attributes->contains(attribute) && !common_attributes().contains(attribute)) {
            unknown.push_back(attribute);
        }
    }
    std::sort(unknown.begin(), unknown.end());

    for (const std::string& attribute : unknown) {
        std::string message = where(n) + "<" + n.tag + "> has no attribute '" + attribute + "'";
        if (const std::string suggestion = closest(attribute, *entry.attributes); !suggestion.empty()) {
            message += " (did you mean '" + suggestion + "'?)";
        }

        if (strict_) {
            LOG(message);
            throw SceneError(message);
        }
        LOG("Warning: " + message);
        warnings_.push_back(message);
    }
}

std::unique_ptr<UIElement> UIFactory::build_node(const Node& n) {
    const auto it = map_.find(n.tag);

    if (it == map_.end()) {
        if (n.tag == "Style" || n.tag == "Include") {
            throw SceneError(where(n) + "<" + n.tag +
                             "> must be resolved before building: load the scene with ui::SceneLoader::load, or run "
                             "ui::SceneComposer::compose on the parsed tree");
        }
        throw SceneError(where(n) + "Unknown tag: " + n.tag);
    }

    check_attributes(n, it->second);

    std::unique_ptr<UIElement> el;
    try {
        el = it->second.make(n);

        // "name"/"id" are common to every element (used by
        // UIElement::find_by_name / find_by_id and by ui::Menu), and so
        // are the paint-order and Overlay-placement attributes (visible,
        // z_index, layer, hit_testable, margin, h_align/v_align, offset):
        // they are applied once here rather than in every make_* tag
        // handler, so every tag - including ones registered later through
        // register_tag - understands them.
        add_common_attributes(n, "Node <" + n.tag + ">", el.get());
    } catch (const SceneError&) {
        throw;
    } catch (const std::exception& ex) {
        // The make_* handlers know which attribute is wrong but not
        // where it is; the node does.
        throw SceneError(where(n) + ex.what());
    }

    for (auto& c : n.children) {
        el->add(build_node(*c));
    }

    return el;
}

// Singleton pattern: exactly one UIFactory maps tag names to
// element constructors for the whole program, so every part of the
// engine that parses UI XML shares the same set of registered tags.
// Each registration carries its tag's attribute schema (the common
// attributes are implied); keep it in step with the make_* handler.
UIFactory::UIFactory() {
    register_tag("Element", make_Element, {"width", "height", "flex"});
    register_tag("VBox", make_VBox, {"width", "height", "padding", "spacing", "align", "justify", "flex"});
    register_tag("HBox", make_HBox, {"width", "height", "padding", "spacing", "align", "justify", "flex"});
    register_tag("Menu", make_Menu, {"width", "height", "padding", "spacing", "align", "justify", "flex"});
    register_tag("Overlay", make_Overlay, {"width", "height", "padding", "flex"});
    register_tag("WorldView", make_WorldView, {"width", "height", "flex", "aspect", "zoom", "center", "y_axis_up"});
    register_tag("ScrollView", make_ScrollView,
                 {"width", "height", "padding", "flex", "direction", "scroll_step", "follow_end"});
    register_tag("Text", make_Text,
                 {"guid", "color", "fontSize", "width", "height", "flex", "inner_text", "h_text_align", "v_text_align",
                  "wrap", "ellipsis", "stretch_font", "max_lines", "line_spacing"});
    register_tag("Rectangle", make_Rectangle, {"width", "height", "flex", "fill_color"});
    register_tag("Sprite", make_Sprite, {"guid", "record_name", "record_id", "width", "height", "flex", "tint"});
    register_tag("Animation", make_Animation, {"guid", "width", "height", "flex", "speed", "tint", "playing"});
    register_tag("Button", make_Button,
                 {"width", "height", "flex", "unselected_color", "selected_color", "transition_duration", "selected",
                  "enabled", "disabled_color", "focus_color"});
}
} // namespace sif::ui
