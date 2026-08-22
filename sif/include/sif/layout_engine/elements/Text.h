/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-26
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_TEXT_H
#define RENDER_ENGINE_TEXT_H
#include "UIElement.h"
#include "sif/asset/AssetHandle.h"
#include "sif/asset/internal/Font.h"
#include "sif/internal/Color.h"

namespace sif::ui {
    /**
     * @brief Simple text label UI element.
     *
     */
    class Text : public UIElement {
    public:
        /// @brief Virtual destructor.
        ~Text() override = default;
        explicit Text(asset::AssetHandle<asset::Font> f) : font(std::move(f)) {}

        /**
         * @brief Measures the desired size of the text.
         *
         * Uses a simplified text metric: a fixed width per character
         * and a constant line height.
         *
         * @param available Available space.
         * @return Desired size of the text.
         */
        math::Vector2 measure(const math::Vector2& available) override;
        void append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const override;

        std::string text;

        intrnl::Color color = intrnl::Color(255, 255, 255);

        asset::AssetHandle<asset::Font> font;

        int fontSize = 14;

        bool stretch_font = false;
    };
} // namespace sif::ui

#endif // RENDER_ENGINE_TEXT_H