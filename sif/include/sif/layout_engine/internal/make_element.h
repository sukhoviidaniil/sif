/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-13
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_MAKE_ELEMENT_H
#define RENDER_ENGINE_MAKE_ELEMENT_H

#include "add_attribute.h"
#include "sif/internal/GUID.h"
#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/Font.h"
#include "sif/layout_engine/Node.h"
#include "sif/layout_engine/elements/LinearLayout.h"
#include "sif/layout_engine/elements/Rectangle.h"
#include "sif/layout_engine/elements/Sprite.h"
#include "sif/layout_engine/elements/Animation.h"
#include "sif/layout_engine/elements/Button.h"
#include "sif/layout_engine/elements/Menu.h"
#include "sif/layout_engine/elements/Text.h"
#include "sif/layout_engine/elements/V_HBox.h"


namespace sif::ui {

    /**
     * @brief Reads the required "guid" attribute of a node.
     *
     * Shared by every asset-backed tag (Text/Sprite/Animation) so the
     * "missing/invalid guid" diagnostics read the same everywhere.
     */
    inline intrnl::GUID require_guid(const Node& n, const std::string& from) {
        const std::string attribute = "guid";
        if (!n.attributes.contains(attribute)) {
            missing_attribute(from, attribute);
        }
        try {
            return intrnl::GUID(n.attributes.at(attribute));
        } catch (const std::exception& ex) {
            invalid_attribute(from, attribute, n.attributes.at(attribute), ex.what());
        }
        return intrnl::GUID{}; // unreachable: invalid_attribute always throws
    }

    inline std::unique_ptr<UIElement> make_Element(const Node& n) {
        static std::string from = "Element node";
        auto e = std::make_unique<UIElement>();

        add_attribute_width(n, from, e.get());
        add_attribute_height(n, from, e.get());
        add_attribute_flex(n, from, e.get());
        add_attribute_name(n, e.get());
        add_attribute_id(n, from, e.get());

        return e;
    }

    inline std::unique_ptr<UIElement> make_VBox(const Node& n) {
        static std::string from = "VBox node";

        auto e = std::make_unique<VBox>();
        VBox *ep = e.get();

        add_attribute_width(n, from, ep);
        add_attribute_height(n, from, ep);
        add_attribute_padding(n, from, ep);
        add_attribute_spacing(n, from, ep);
        add_attribute_align(n, from, ep);
        add_attribute_flex(n, from, ep);
        add_attribute_name(n, ep);
        add_attribute_id(n, from, ep);

        return e;
    }

    inline std::unique_ptr<UIElement> make_HBox(const Node& n) {
        static std::string from = "HBox node";

        auto e = std::make_unique<HBox>();
        HBox* ep = e.get();

        add_attribute_width(n, from, ep);
        add_attribute_height(n, from, ep);
        add_attribute_padding(n, from, ep);
        add_attribute_spacing(n, from, ep);
        add_attribute_align(n, from, ep);
        add_attribute_flex(n, from, ep);
        add_attribute_name(n, ep);
        add_attribute_id(n, from, ep);

        return e;
    }

    inline std::unique_ptr<UIElement> make_Rectangle(const Node& n) {
        static std::string from = "Rectangle node";
        auto e = std::make_unique<Rectangle>();

        add_attribute_width(n, from, e.get());
        add_attribute_height(n, from, e.get());
        add_attribute_flex(n, from, e.get());
        add_attribute_name(n, e.get());
        add_attribute_id(n, from, e.get());
        // ReSharper disable once CppJoinDeclarationAndAssignment
        std::string attrbt;

        attrbt = "fill_color";
        if (n.attributes.contains(attrbt)) {
            try {
                e->fill_color = intrnl::Color(n.attributes.at(attrbt));
            } catch (const std::exception& ex) {
                invalid_attribute(from, attrbt, n.attributes.at(attrbt), ex.what());
            }
        }

        return e;
    }

    inline std::unique_ptr<UIElement> make_Text(const Node& n) {
        static std::string from = "Text node";
        // ReSharper disable once CppJoinDeclarationAndAssignment
        std::string attrbt;

        const intrnl::GUID guid = require_guid(n, from);

        asset::AssetRegistry::instance().request(guid);
        asset::AssetHandle<asset::Font> font = asset::AssetRegistry::instance().get<asset::Font>(guid);
        auto e = std::make_unique<Text>(std::move(font));

        attrbt = "color";
        if (n.attributes.contains(attrbt)) {
            try {
                e->color = intrnl::Color(n.attributes.at(attrbt));
            } catch (const std::exception& ex) {
                invalid_attribute(from, attrbt, n.attributes.at(attrbt), ex.what());
            }
        }

        attrbt = "fontSize";
        if (n.attributes.contains(attrbt)) {
            try {
                e->fontSize = std::stoi(n.attributes.at("fontSize"));
            } catch (const std::exception& ex) {
                invalid_attribute(from, attrbt, n.attributes.at(attrbt), ex.what());
            }
        }

        add_attribute_width(n, from, e.get());
        add_attribute_height(n, from, e.get());
        add_attribute_flex(n, from, e.get());
        add_attribute_name(n, e.get());
        add_attribute_id(n, from, e.get());

        attrbt = "inner_text";
        if (n.attributes.contains(attrbt)) {
            try {
                e->text = n.attributes.at(attrbt);
            } catch (const std::exception& ex) {
                invalid_attribute(from, attrbt, n.attributes.at(attrbt), ex.what());
            }
        }

        return e;
    }

    inline std::unique_ptr<UIElement> make_Sprite(const Node& n) {
        static std::string from = "Sprite node";

        // A <Sprite> is identified by the same GUID mechanism as every
        // other asset reference (see reference_serialization: authoring
        // files use asset_name, the serialized ones carry guid), rather
        // than by a free-form name that nothing ever resolved.
        const intrnl::GUID guid = require_guid(n, from);

        asset::AssetRegistry& registry = asset::AssetRegistry::instance();
        registry.request(guid);

        const asset::AssetType kind = registry.type_of(guid);
        if (kind != asset::AssetType::SpriteSingle &&
            kind != asset::AssetType::SpriteAtlas &&
            kind != asset::AssetType::SpriteGrid) {
            throw std::runtime_error(
                from + ": asset " + guid.string() + " is a " + asset::to_string(kind) +
                ", which <Sprite> cannot draw (expected SpriteSingle/SpriteAtlas/SpriteGrid)"
            );
            }

        // Sub-sprite selection: either a numeric record_id, or a
        // human-readable record_name resolved through the asset's
        // metadata (record_id_to_name).
        intrnl::RecordID record_id{0};
        if (n.attributes.contains("record_name")) {
            try {
                record_id = registry.record_id_of(guid, n.attributes.at("record_name"));
            } catch (const std::exception& ex) {
                invalid_attribute(from, "record_name", n.attributes.at("record_name"), ex.what());
            }
        } else if (n.attributes.contains("record_id")) {
            try {
                record_id = intrnl::RecordID(static_cast<std::uint32_t>(std::stoul(n.attributes.at("record_id"))));
            } catch (const std::exception& ex) {
                invalid_attribute(from, "record_id", n.attributes.at("record_id"), ex.what());
            }
        }

        auto e = std::make_unique<Sprite>(registry.get<void>(guid), kind, record_id);
        Sprite* ep = e.get();

        add_attribute_width(n, from, ep);
        add_attribute_height(n, from, ep);
        add_attribute_flex(n, from, ep);
        add_attribute_name(n, ep);
        add_attribute_id(n, from, ep);

        if (n.attributes.contains("tint")) {
            try {
                ep->tint = intrnl::Color(n.attributes.at("tint"));
            } catch (const std::exception& ex) {
                invalid_attribute(from, "tint", n.attributes.at("tint"), ex.what());
            }
        }

        return e;
    }

    inline std::unique_ptr<UIElement> make_Animation(const Node& n) {
        static std::string from = "Animation node";

        const intrnl::GUID guid = require_guid(n, from);

        asset::AssetRegistry& registry = asset::AssetRegistry::instance();
        registry.request(guid);

        const asset::AssetType kind = registry.type_of(guid);
        if (kind != asset::AssetType::PrimitiveAnimation) {
            throw std::runtime_error(
                from + ": asset " + guid.string() + " is a " + asset::to_string(kind) +
                ", expected PrimitiveAnimation"
            );
        }

        auto e = std::make_unique<Animation>(registry.get<asset::PrimitiveAnimation>(guid));
        Animation* ep = e.get();

        add_attribute_width(n, from, ep);
        add_attribute_height(n, from, ep);
        add_attribute_flex(n, from, ep);
        add_attribute_name(n, ep);
        add_attribute_id(n, from, ep);

        if (n.attributes.contains("speed")) {
            try {
                ep->speed = std::stof(n.attributes.at("speed"));
            } catch (const std::exception& ex) {
                invalid_attribute(from, "speed", n.attributes.at("speed"), ex.what());
            }
        }

        if (n.attributes.contains("tint")) {
            try {
                ep->tint = intrnl::Color(n.attributes.at("tint"));
            } catch (const std::exception& ex) {
                invalid_attribute(from, "tint", n.attributes.at("tint"), ex.what());
            }
        }

        if (n.attributes.contains("playing") && n.attributes.at("playing") == "false") {
            ep->pause();
        }

        return e;
    }

    inline std::unique_ptr<UIElement> make_Button(const Node& n) {
        static std::string from = "Button node";
        auto e = std::make_unique<Button>();
        Button* ep = e.get();

        add_attribute_width(n, from, ep);
        add_attribute_height(n, from, ep);
        add_attribute_flex(n, from, ep);
        add_attribute_name(n, ep);
        add_attribute_id(n, from, ep);

        std::string attrbt;

        attrbt = "unselected_color";
        if (n.attributes.contains(attrbt)) {
            try {
                e->unselected_color = intrnl::Color(n.attributes.at(attrbt));
            } catch (const std::exception& ex) {
                invalid_attribute(from, attrbt, n.attributes.at(attrbt), ex.what());
            }
        }

        attrbt = "selected_color";
        if (n.attributes.contains(attrbt)) {
            try {
                e->selected_color = intrnl::Color(n.attributes.at(attrbt));
            } catch (const std::exception& ex) {
                invalid_attribute(from, attrbt, n.attributes.at(attrbt), ex.what());
            }
        }

        attrbt = "transition_duration";
        if (n.attributes.contains(attrbt)) {
            try {
                e->transition_duration_seconds = std::stof(n.attributes.at(attrbt));
            } catch (const std::exception& ex) {
                invalid_attribute(from, attrbt, n.attributes.at(attrbt), ex.what());
            }
        }

        attrbt = "selected";
        if (n.attributes.contains(attrbt) && n.attributes.at(attrbt) == "true") {
            e->select();
        }

        return e;
    }

    inline std::unique_ptr<UIElement> make_Menu(const Node& n) {
        static std::string from = "Menu node";

        auto e = std::make_unique<Menu>();
        Menu* ep = e.get();

        add_attribute_width(n, from, ep);
        add_attribute_height(n, from, ep);
        add_attribute_padding(n, from, ep);
        add_attribute_spacing(n, from, ep);
        add_attribute_align(n, from, ep);
        add_attribute_flex(n, from, ep);
        add_attribute_name(n, ep);
        add_attribute_id(n, from, ep);

        return e;
    }
}

#endif