/***************************************************************
 * Project:          Render_Engine
 * File:             Sprite.h
 *
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-24
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_AST_RI_SPRITE_H
#define RENDER_ENGINE_AST_RI_SPRITE_H

#include "RenderNode.h"

#include "sif/asset/AssetHandle.h"
#include "sif/asset/internal/AssetType.h"
#include "sif/internal/Color.h"

namespace sif::rnd {
    /**
     * @brief One textured quad to draw this frame.
     *
     * The UI element that emits this item already knows the concrete
     * asset type (SpriteSingle / SpriteAtlas / SpriteGrid /
     * PrimitiveAnimation) and has resolved which part of the texture
     * to show, so the backend only ever has to answer one question:
     * "give me the texture behind this handle". That question is
     * answered by casting the handle's payload to the backend's own
     * subclass, which is why `kind` travels with the handle - the
     * type-erased AssetHandle<void> alone does not say which subclass
     * is on the other side.
     *
     * `src_rect` with width == 0 or height == 0 means "the whole
     * texture" (used by SpriteSingle).
     */
    struct Sprite : RenderNode {
        void accept(RenderVisitor& v) const override { v.visit(*this); }

        /// Type-erased handle to the loaded asset (never dereferenced by the engine).
        asset::AssetHandle<void> asset;

        /// Which concrete asset class `asset` points at; drives the backend cast.
        asset::AssetType kind = asset::AssetType::Undefined;

        /// Sub-rect of the texture to draw; zero-sized means "whole texture".
        intrnl::Rect src_rect{};

        /// Multiplied with the texture's own colors (255,255,255,255 = unchanged).
        intrnl::Color tint{};
    };
} // namespace sif::rnd

#endif // RENDER_ENGINE_AST_RI_SPRITE_H
