/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-24
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_RENDERNODE_H
#define RENDER_ENGINE_RENDERNODE_H

#include <optional>

#include "sif/internal/Rect.h"

namespace sif::rnd {

struct Text;
struct Rectangle;
struct Sprite;

/**
 * @brief Visitor pattern: double-dispatches a RenderNode to the
 * concrete render call matching its real type (Text/Rectangle/
 * Sprite), without RenderNode/RenderFrame needing to know anything
 * about *how* a Text is drawn versus a Rectangle. Adding a new
 * backend (an alternative to SFML_Renderer) means implementing this
 * interface once; adding a new render item type means adding one
 * visit() overload here and in every backend - the tradeoff the
 * Visitor pattern makes explicit either way.
 */
struct RenderVisitor {
    virtual ~RenderVisitor() = default;

    virtual void visit(const Text&) = 0;
    virtual void visit(const Rectangle&) = 0;
    virtual void visit(const Sprite&) = 0;
};

struct RenderNode {
    virtual ~RenderNode() = default;

    virtual void accept(RenderVisitor& v) const = 0;

    /// Element dimensions
    intrnl::Rect rect{};

    /**
     * @brief Paint layer this item belongs to (see rnd::layer).
     *
     * Higher layers are drawn over lower ones; within one layer the
     * submission order is kept. Normally stamped by
     * RenderFrame::submit from the current LayerScope rather than set
     * by hand, so an element never needs to know which layer its
     * ancestors put it on.
     */
    int layer = 0;

    /**
     * @brief Screen-space rect this item is clipped to (nullopt = not clipped).
     *
     * Stamped by RenderFrame::submit from the current ClipScope, the
     * same way layer is, so an element never needs to know which
     * ancestor clips it. A backend draws only the part of the item that
     * lies inside it; an empty clip means the item is not visible at all.
     */
    std::optional<intrnl::Rect> clip;
};
} // namespace sif::rnd

#endif // RENDER_ENGINE_RENDERNODE_H