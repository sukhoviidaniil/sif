/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-01-17
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "SFML_Renderer.h"

#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Text.hpp"

#include "SFMLAssets.h"

#include "sif/event/events/window.hpp"

namespace sif::sfml {
    namespace {
        sf::Color to_sfml(const intrnl::Color& c) {
            return {c.r, c.g, c.b, c.a};
        }
    }

    SFML_Renderer::~SFML_Renderer() = default;

    SFML_Renderer::SFML_Renderer(const ast::RB_Config &info)
        : window_(sf::VideoMode(info.window_width, info.window_height), info.window_name) {
        sf::View view(sf::FloatRect(0.f, 0.f,
                                    static_cast<float>(info.window_width),
                                    static_cast<float>(info.window_height)));
        window_.setView(view);

        // Capping the frame rate is the one form of waiting the
        // assignment allows; the simulation itself still runs on the
        // measured deltaTime, never on a fixed step.
        if (info.fps > 0) {
            window_.setFramerateLimit(static_cast<unsigned int>(info.fps));
        }
    }

    void SFML_Renderer::track_local(const std::shared_ptr<event::Event_Bus>& /*bus*/) {
        // No per-scene events are consumed by the renderer itself yet;
        // the hook exists so a scene-scoped bus can be attached later
        // without changing App.
    }

    void SFML_Renderer::track_global(const std::shared_ptr<event::Event_Bus> &bus) {
        using namespace event::window;

        track(
            bus->subscribe<Window_Closed>(
                [this](const Window_Closed&) {
                    window_.close();
                }
            )
        );

        track(
            bus->subscribe<Window_Resized>(
                [this](const Window_Resized& e) {
                    // Keep one screen pixel equal to one layout unit
                    // after a resize, otherwise the whole UI would be
                    // stretched by SFML instead of re-laid out.
                    const sf::View v(sf::FloatRect(0.f, 0.f,
                                                   static_cast<float>(e.width),
                                                   static_cast<float>(e.height)));
                    window_.setView(v);
                }
            )
        );
    }

    math::Vector2 SFML_Renderer::screen_size() const {
        return {
            static_cast<float>(window_.getSize().x),
            static_cast<float>(window_.getSize().y)
        };
    }

    void SFML_Renderer::render(const rnd::RenderFrame &graph) {
        window_.clear(to_sfml(clear_color));
        for (const auto& item : graph.constant_items) {
            item->accept(*this);
        }
        for (const auto& item : graph.temp_items) {
            item->accept(*this);
        }
        window_.display();
    }

    bool SFML_Renderer::poll_event(sf::Event &e) {
        return window_.pollEvent(e);
    }

    bool SFML_Renderer::is_open() const {
        return window_.isOpen();
    }

    const sf::Texture* SFML_Renderer::texture_of(const rnd::Sprite &r) {
        void* raw = r.asset.get();
        if (raw == nullptr) {
            return nullptr;
        }

        switch (r.kind) {
            case asset::AssetType::SpriteSingle:
                return &static_cast<const SFMLSpriteAsset*>(
                    static_cast<const asset::Sprite*>(raw))->texture();
            case asset::AssetType::SpriteAtlas:
                return &static_cast<const SFMLSpriteAtlasAsset*>(
                    static_cast<const asset::SpriteAtlas*>(raw))->texture();
            case asset::AssetType::SpriteGrid:
                return &static_cast<const SFMLSpriteGridAsset*>(
                    static_cast<const asset::SpriteGrid*>(raw))->texture();
            case asset::AssetType::PrimitiveAnimation:
                return &static_cast<const SFMLPrimitiveAnimationAsset*>(
                    static_cast<const asset::PrimitiveAnimation*>(raw))->texture();
            default:
                return nullptr;
        }
    }

    void SFML_Renderer::visit(const rnd::Text & r) {
        const asset::Font* font_asset = r.font.get();
        if (font_asset == nullptr) {
            return; // still loading: skip this frame rather than crash
        }
        const auto* sfml_font = static_cast<const SFMLFontAsset*>(font_asset);

        sf::Text text;
        text.setFont(sfml_font->font());
        text.setString(r.text);
        text.setFillColor(to_sfml(r.color));

        const intrnl::Rect& rect = r.rect;

        if (r.size > 0) {
            text.setCharacterSize(static_cast<unsigned int>(r.size));
            text.setPosition(rect.x, rect.y);
        } else {
            // Stretch the text across the whole rect.
            text.setCharacterSize(30); // base size for the scale computation

            const sf::FloatRect bounds = text.getLocalBounds();
            if (bounds.width <= 0.f || bounds.height <= 0.f) {
                return; // empty string: nothing to scale against
            }

            text.setOrigin(bounds.left, bounds.top);
            text.setScale(rect.width / bounds.width, rect.height / bounds.height);
            text.setPosition(rect.x, rect.y);
        }

        window_.draw(text);
    }

    void SFML_Renderer::visit(const rnd::Rectangle & r) {
        sf::RectangleShape rect;

        rect.setPosition(r.rect.x, r.rect.y);
        rect.setSize({r.rect.width, r.rect.height});
        rect.setFillColor(to_sfml(r.color));

        if (r.border_width > 0) {
            rect.setOutlineThickness(static_cast<float>(r.border_width));
            rect.setOutlineColor(to_sfml(r.border_color));
        } else {
            rect.setOutlineThickness(0.f);
        }

        window_.draw(rect);
    }

    void SFML_Renderer::visit(const rnd::Sprite & r) {
        const sf::Texture* texture = texture_of(r);
        if (texture == nullptr) {
            return;
        }

        sf::Sprite sprite(*texture);

        // A zero-sized source rect means "the whole texture" (that is
        // how SpriteSingle is expressed); atlases, grids and animation
        // frames all arrive here already resolved to one sub-rect.
        sf::FloatRect source{
            0.f, 0.f,
            static_cast<float>(texture->getSize().x),
            static_cast<float>(texture->getSize().y)
        };
        if (r.src_rect.width > 0.f && r.src_rect.height > 0.f) {
            source = {r.src_rect.x, r.src_rect.y, r.src_rect.width, r.src_rect.height};
            sprite.setTextureRect(sf::IntRect(
                static_cast<int>(source.left),
                static_cast<int>(source.top),
                static_cast<int>(source.width),
                static_cast<int>(source.height)
            ));
        }

        if (source.width <= 0.f || source.height <= 0.f) {
            return;
        }

        sprite.setPosition(r.rect.x, r.rect.y);
        if (r.rect.width > 0.f && r.rect.height > 0.f) {
            sprite.setScale(r.rect.width / source.width, r.rect.height / source.height);
        }
        sprite.setColor(to_sfml(r.tint));

        window_.draw(sprite);
    }
}
