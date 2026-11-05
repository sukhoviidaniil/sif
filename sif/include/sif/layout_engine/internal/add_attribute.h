/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-02-08
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_ADD_ATTRIBUTE_H
#define RENDER_ENGINE_ADD_ATTRIBUTE_H
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>

#include "attribute_check.h"
#include "sif/internal/Alignment.h"
#include "sif/internal/Size.h"
#include "sif/layout_engine/Node.h"
#include "sif/layout_engine/elements/LinearLayout.h"
#include "sif/layout_engine/elements/UIElement.h"
#include "sif/math/Vector2.h"
#include "sif/render/Layer.h"

namespace sif::ui {

inline void add_attribute_width(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "width";
    if (n.attributes.contains(attribute)) {
        try {
            element->width = intrnl::Size(n.attributes.at(attribute));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

inline void add_attribute_height(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "height";
    if (n.attributes.contains(attribute)) {
        try {
            element->height = intrnl::Size(n.attributes.at(attribute));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

/**
 * @brief Applies the optional "name" attribute, used by UIElement::find_by_name.
 *
 * Common to every element type (unlike width/height/padding, which
 * only some elements interpret), so UIFactory::build applies this
 * once for every tag rather than each make_* function repeating it.
 */
inline void add_attribute_name(const Node& n, UIElement* element) {
    const std::string attribute = "name";
    if (n.attributes.contains(attribute)) {
        element->name = n.attributes.at(attribute);
    }
}

/**
 * @brief Applies the optional "id" attribute, used by UIElement::find_by_id
 * and by ui::Menu::set_active.
 *
 * Common to every element type, applied once by UIFactory::build.
 */
inline void add_attribute_id(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "id";
    if (n.attributes.contains(attribute)) {
        try {
            element->id = intrnl::RecordID(static_cast<std::uint32_t>(std::stoul(n.attributes.at(attribute))));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

inline void add_attribute_padding(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "padding";
    if (n.attributes.contains(attribute)) {
        try {
            const float a = std::stof(n.attributes.at(attribute));
            element->padding = {a, a};
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

/**
 * @brief Applies the optional "flex" attribute (grow factor inside
 * a LinearLayout).
 *
 * Without this, a scene author could only size children in absolute
 * pixels or percentages: the flex machinery existed in
 * LinearLayout::layout but was unreachable from XML.
 */
inline void add_attribute_flex(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "flex";
    if (n.attributes.contains(attribute)) {
        try {
            element->flex = std::stof(n.attributes.at(attribute));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

/**
 * @brief Parses an intrnl::Align value (Start | Center | End | Stretch).
 *
 * Shared by the container-level "align" (LinearLayout) and the
 * per-child "h_align"/"v_align" (read by Overlay), so the spelling
 * and the error message are the same everywhere.
 */
inline intrnl::Align parse_align(const std::string& from, const std::string& attribute, const std::string& value) {
    if (value == "Start")
        return intrnl::Align::Start;
    if (value == "Center")
        return intrnl::Align::Center;
    if (value == "End")
        return intrnl::Align::End;
    if (value == "Stretch")
        return intrnl::Align::Stretch;
    invalid_attribute(from, attribute, value, "expected Start|Center|End|Stretch");
    return intrnl::Align::Start; // unreachable: invalid_attribute always throws
}

/**
 * @brief Parses "a" (both components a) or "x|y", the "|" separator
 * matching intrnl::Size and intrnl::Color.
 *
 * @throws std::invalid_argument on anything else.
 */
inline math::Vector2 parse_vector2(const std::string_view str) {
    const auto parse_float = [](const std::string_view part) {
        const std::string tmp(part);
        char* end = nullptr;
        const float v = std::strtof(tmp.c_str(), &end);
        if (tmp.empty() || end != tmp.c_str() + tmp.size()) {
            throw std::invalid_argument("expected a number, or two numbers as x|y");
        }
        return v;
    };

    const auto sep = str.find('|');
    if (sep == std::string_view::npos) {
        const float a = parse_float(str);
        return {a, a};
    }
    return {parse_float(str.substr(0, sep)), parse_float(str.substr(sep + 1))};
}

/**
 * @brief Parses "true" / "false" strictly.
 *
 * Strict on purpose: a typo such as visible="flase" silently meaning
 * "true" is exactly the kind of scene bug that is hard to see.
 */
inline bool parse_bool(const std::string& from, const std::string& attribute, const std::string& value) {
    if (value == "true")
        return true;
    if (value == "false")
        return false;
    invalid_attribute(from, attribute, value, "expected true|false");
    return false; // unreachable: invalid_attribute always throws
}

/**
 * @brief Applies the optional "align" attribute of a LinearLayout
 * (Start | Center | End | Stretch).
 */
inline void add_attribute_align(const Node& n, const std::string& from, LinearLayout* element) {
    const std::string attribute = "align";
    if (n.attributes.contains(attribute)) {
        element->align = parse_align(from, attribute, n.attributes.at(attribute));
    }
}

/**
 * @brief Applies the optional "justify" attribute of a LinearLayout
 * (Start | Center | End | SpaceBetween | SpaceAround | SpaceEvenly).
 */
inline void add_attribute_justify(const Node& n, const std::string& from, LinearLayout* element) {
    const std::string attribute = "justify";
    if (!n.attributes.contains(attribute)) {
        return;
    }
    const std::string& value = n.attributes.at(attribute);
    if (value == "Start")
        element->justify = intrnl::Justify::Start;
    else if (value == "Center")
        element->justify = intrnl::Justify::Center;
    else if (value == "End")
        element->justify = intrnl::Justify::End;
    else if (value == "SpaceBetween")
        element->justify = intrnl::Justify::SpaceBetween;
    else if (value == "SpaceAround")
        element->justify = intrnl::Justify::SpaceAround;
    else if (value == "SpaceEvenly")
        element->justify = intrnl::Justify::SpaceEvenly;
    else
        invalid_attribute(from, attribute, value, "expected Start|Center|End|SpaceBetween|SpaceAround|SpaceEvenly");
}

inline void add_attribute_spacing(const Node& n, const std::string& from, LinearLayout* element) {
    const std::string attribute = "spacing";
    if (n.attributes.contains(attribute)) {
        try {
            const float a = std::stof(n.attributes.at(attribute));
            element->spacing = a;
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

// ===== Attributes common to every element =====
//
// Applied once by UIFactory::build for every tag (like name/id), not
// per make_* function: they describe how an element takes part in
// its parent's layout and in the paint order, which is independent
// of what kind of element it is.

/**
 * @brief Applies the optional "visible" attribute (true | false).
 *
 * Lets a scene declare an overlay that starts hidden (a pause panel,
 * a dialog) and is shown later by setting UIElement::visible.
 */
inline void add_attribute_visible(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "visible";
    if (n.attributes.contains(attribute)) {
        element->visible = parse_bool(from, attribute, n.attributes.at(attribute));
    }
}

/**
 * @brief Applies the optional "z_index" attribute (paint order among
 * siblings, see UIElement::paint_order).
 */
inline void add_attribute_z_index(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "z_index";
    if (n.attributes.contains(attribute)) {
        try {
            element->z_index = std::stoi(n.attributes.at(attribute));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

/**
 * @brief Applies the optional "layer" attribute: a name from
 * rnd::layer (Background | Content | Popup | Modal | Tooltip) or an
 * integer.
 */
inline void add_attribute_layer(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "layer";
    if (n.attributes.contains(attribute)) {
        try {
            element->layer = rnd::layer_from_string(n.attributes.at(attribute));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

/**
 * @brief Applies the optional "hit_testable" attribute (true | false,
 * see UIElement::hit_testable).
 */
/**
 * @brief Applies the optional "clip" attribute (true | false, see UIElement::clip).
 */
inline void add_attribute_clip(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "clip";
    if (n.attributes.contains(attribute)) {
        element->clip = parse_bool(from, attribute, n.attributes.at(attribute));
    }
}

/**
 * @brief Applies the optional "focusable" attribute (true | false, see UIElement::focusable).
 */
inline void add_attribute_focusable(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "focusable";
    if (n.attributes.contains(attribute)) {
        element->focusable = parse_bool(from, attribute, n.attributes.at(attribute));
    }
}

inline void add_attribute_hit_testable(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "hit_testable";
    if (n.attributes.contains(attribute)) {
        element->hit_testable = parse_bool(from, attribute, n.attributes.at(attribute));
    }
}

/**
 * @brief Applies the optional "margin" attribute ("a" or "x|y").
 *
 * UIElement::margin was already honoured by the base UIElement
 * layout, but nothing could set it from XML; Overlay uses it to inset
 * a child from the edge it is aligned to.
 */
inline void add_attribute_margin(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "margin";
    if (n.attributes.contains(attribute)) {
        try {
            element->margin = parse_vector2(n.attributes.at(attribute));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

/**
 * @brief Applies the optional "h_align" / "v_align" attributes
 * (placement of a child inside a parent Overlay).
 */
inline void add_attribute_overlay_align(const Node& n, const std::string& from, UIElement* element) {
    if (n.attributes.contains("h_align")) {
        element->h_align = parse_align(from, "h_align", n.attributes.at("h_align"));
    }
    if (n.attributes.contains("v_align")) {
        element->v_align = parse_align(from, "v_align", n.attributes.at("v_align"));
    }
}

/**
 * @brief Applies the optional "offset" attribute ("a" or "x|y"; shift
 * of a child inside a parent Overlay, applied after alignment).
 */
inline void add_attribute_offset(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "offset";
    if (n.attributes.contains(attribute)) {
        try {
            element->offset = parse_vector2(n.attributes.at(attribute));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

/**
 * @brief Parses "x|y|width|height" into a Rect.
 *
 * @throws std::invalid_argument on anything else.
 */
inline intrnl::Rect parse_rect(const std::string_view str) {
    float v[4] = {};
    std::size_t start = 0;
    for (int i = 0; i < 4; ++i) {
        const std::size_t sep = i < 3 ? str.find('|', start) : str.size();
        if (sep == std::string_view::npos) {
            throw std::invalid_argument("expected x|y|width|height");
        }
        const std::string part(str.substr(start, sep - start));
        char* end = nullptr;
        v[i] = std::strtof(part.c_str(), &end);
        if (part.empty() || end != part.c_str() + part.size()) {
            throw std::invalid_argument("expected four numbers as x|y|width|height");
        }
        start = sep + 1;
    }
    if (start <= str.size()) {
        throw std::invalid_argument("expected exactly four numbers as x|y|width|height");
    }
    return {v[0], v[1], v[2], v[3]};
}

/**
 * @brief Applies the optional "world_rect" attribute (x|y|width|height in
 * world units, read by a parent WorldView).
 */
inline void add_attribute_world_rect(const Node& n, const std::string& from, UIElement* element) {
    const std::string attribute = "world_rect";
    if (n.attributes.contains(attribute)) {
        try {
            element->world_rect = parse_rect(n.attributes.at(attribute));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
    }
}

/**
 * @brief Applies every attribute common to all elements.
 *
 * The single place UIFactory::build calls, so a new common attribute
 * is added here once instead of in every tag handler.
 */
inline void add_common_attributes(const Node& n, const std::string& from, UIElement* element) {
    add_attribute_name(n, element);
    add_attribute_id(n, from, element);
    add_attribute_visible(n, from, element);
    add_attribute_z_index(n, from, element);
    add_attribute_layer(n, from, element);
    add_attribute_hit_testable(n, from, element);
    add_attribute_clip(n, from, element);
    add_attribute_focusable(n, from, element);
    add_attribute_margin(n, from, element);
    add_attribute_overlay_align(n, from, element);
    add_attribute_offset(n, from, element);
    add_attribute_world_rect(n, from, element);
}

} // namespace sif::ui

#endif // RENDER_ENGINE_ADD_ATTRIBUTE_H