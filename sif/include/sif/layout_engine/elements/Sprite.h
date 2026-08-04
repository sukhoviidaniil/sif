/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-27
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_UI_SPRITEELEMENT_H
#define RENDER_ENGINE_UI_SPRITEELEMENT_H

#include "UIElement.h"

#include "sif/asset/AssetHandle.h"
#include "sif/asset/internal/AssetType.h"
#include "sif/internal/Color.h"
#include "sif/internal/RecordID.h"

namespace sif::ui {
    /**
     * @brief Draws one still image: a whole SpriteSingle, one entry of
     * a SpriteAtlas, or one cell of a SpriteGrid.
     *
     * The element holds a typeless AssetHandle plus the AssetType it
     * was created for; that pair is enough for it to resolve the
     * sub-rect to show (see resolve_src_rect) entirely inside the
     * engine, so the render backend never has to know about atlases
     * or grids - it only receives "this texture, this rect".
     *
     * Until the referenced asset finishes loading, the element simply
     * emits nothing (while still occupying its layout space), so a
     * slow asset never turns into a crash or a garbage draw.
     */
    class Sprite : public UIElement {
    public:
        Sprite(asset::AssetHandle<void> handle,
               asset::AssetType kind,
               intrnl::RecordID record_id = intrnl::RecordID{0});

        math::Vector2 measure(const math::Vector2& available) override;

        void append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const override;

        /// @brief Which sub-sprite/cell of an atlas/grid to show. Ignored for SpriteSingle.
        void set_record_id(intrnl::RecordID record_id);
        [[nodiscard]] intrnl::RecordID record_id() const;

        /// @brief True once the underlying asset is loaded and drawable.
        [[nodiscard]] bool ready() const;

        /// @brief Multiplied with the texture's colors when drawn.
        intrnl::Color tint{};

    private:
        /**
         * @brief Resolves which part of the texture this element shows.
         *
         * @return A zero-sized rect for SpriteSingle ("whole texture"),
         * or the atlas entry / grid cell for the current record id.
         * Out-of-range record ids fall back to the whole texture rather
         * than throwing out of a render pass.
         */
        [[nodiscard]] intrnl::Rect resolve_src_rect() const;

        asset::AssetHandle<void> asset_;
        asset::AssetType kind_;
        intrnl::RecordID record_id_;
    };
}

#endif //RENDER_ENGINE_UI_SPRITEELEMENT_H
