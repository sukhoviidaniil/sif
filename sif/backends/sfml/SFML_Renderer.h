/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2025-12-06
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SFML_SFML_RENDERER_H
#define RENDER_ENGINE_SFML_SFML_RENDERER_H

#include "SFML/Graphics/RenderWindow.hpp"

#include "ISFML_Event_Source.h"

#include "sif/render/RB_Config.h"
#include "sif/render/Renderer.h"
#include "sif/render/elements/Rectangle.h"
#include "sif/render/elements/Sprite.h"
#include "sif/render/elements/Text.h"

namespace sif::sfml {
    /**
     * @brief SFML implementation of the engine's render backend.
     *
     * Owns the window, so it doubles as the window's event source
     * (ISFML_Event_Source) - see the note there on why that is not a
     * separation-of-concerns violation.
     *
     * Every visit() below is deliberately dumb: it receives a fully
     * resolved render item (final rect, final sub-rect, final colour)
     * and turns it into SFML calls. No layout, no asset lookup, no
     * animation timing happens here, which is what keeps "swap SFML for
     * something else" a matter of writing one more class.
     */
    class SFML_Renderer final : public rnd::Renderer, public ISFML_Event_Source {
    public:
        ~SFML_Renderer() override;
        explicit SFML_Renderer(const ast::RB_Config& info);

        void track_local(const std::shared_ptr<event::Event_Bus>& bus) override;
        void track_global(const std::shared_ptr<event::Event_Bus>& bus) override;

        [[nodiscard]] math::Vector2 screen_size() const override;

        void render(const rnd::RenderFrame& graph) override;

        bool poll_event(sf::Event& e) override;

        /// @brief True while the underlying window is still open.
        [[nodiscard]] bool is_open() const;

        /// @brief Background colour used by render() before drawing a frame.
        intrnl::Color clear_color{18, 18, 24};

    private:
        void visit(const rnd::Text& r) override;
        void visit(const rnd::Rectangle& r) override;
        void visit(const rnd::Sprite& r) override;

        /**
         * @brief Recovers the SFML texture behind a render item.
         *
         * rnd::Sprite carries a type-erased handle plus the AssetType
         * it was built for; that pair is enough to static-cast back to
         * the concrete backend class (see SFMLAssets.h for why this is
         * well defined) without a single dynamic_cast.
         *
         * @return nullptr if the asset is not loaded, or its type is
         * not something this backend can draw.
         */
        [[nodiscard]] static const sf::Texture* texture_of(const rnd::Sprite& r);

        sf::RenderWindow window_;
    };
}

#endif //RENDER_ENGINE_SFML_SFML_RENDERER_H
