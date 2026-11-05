/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2025-12-24
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_AST_RENDERFRAME_H
#define RENDER_ENGINE_AST_RENDERFRAME_H

#include <memory>
#include <optional>
#include <vector>

#include "elements/RenderNode.h"
#include "sif/render/Layer.h"

namespace sif::rnd {
/**
 * @brief Everything one frame draws, collected before any drawing starts.
 *
 * constant_items / temp_items are a *caching* distinction (kept
 * between frames vs. rebuilt every frame), not a z-order: which item
 * covers which is decided by each item's RenderNode::layer and, within
 * a layer, by submission order - see draw_order(). Backends therefore
 * never iterate the two buckets themselves; they draw draw_order().
 */
struct RenderFrame {
    std::vector<std::unique_ptr<RenderNode>> constant_items;
    std::vector<std::unique_ptr<RenderNode>> temp_items;

    /**
     * @brief Appends an item to temp_items, stamped with the current
     * layer and clip.
     *
     * The single entry point UI elements use: the element only says
     * "draw this", and which layer that lands on comes from whatever
     * LayerScope its ancestors opened (see ui::UIElement::render). An
     * item pushed into temp_items directly still draws - on the layer
     * it already carries, 0 by default - so existing code keeps working.
     */
    void submit(std::unique_ptr<RenderNode> item);

    /// @brief Layer that submit() currently stamps on new items.
    [[nodiscard]] int current_layer() const;

    /// @brief Clip rect that submit() currently stamps on new items (nullopt = none).
    [[nodiscard]] const std::optional<intrnl::Rect>& current_clip() const;

    /**
     * @brief Every item of the frame, in the order it must be drawn.
     *
     * A stable sort by RenderNode::layer over constant_items followed
     * by temp_items: items on a higher layer come later (are drawn on
     * top), and items on the same layer keep their relative order, with
     * the constant bucket first. When every item is on one layer - the
     * case for any scene that never asks for a layer - this is exactly
     * the old "constant_items, then temp_items" order, so introducing
     * layers changes nothing for existing scenes.
     *
     * The returned pointers are non-owning and valid until the frame
     * is modified.
     */
    [[nodiscard]] std::vector<const RenderNode*> draw_order() const;

private:
    friend class LayerScope;
    friend class ClipScope;

    int current_layer_ = layer::Content;
    std::optional<intrnl::Rect> current_clip_;
};

/**
 * @brief RAII scope that switches the layer RenderFrame::submit stamps.
 *
 * Same shape as diag::LogScope: the constructor enters the new layer,
 * the destructor restores the previous one. Scoping (rather than a
 * plain setter) is what makes layers *inherited*: an element that sets
 * a layer opens a scope around its own and its whole subtree's
 * submission, a nested element can open its own scope inside, and
 * leaving the nested subtree puts the outer layer back automatically -
 * including when the subtree throws.
 */
class LayerScope {
public:
    LayerScope(RenderFrame& frame, int layer);
    ~LayerScope();

    LayerScope(const LayerScope&) = delete;
    LayerScope& operator=(const LayerScope&) = delete;

private:
    RenderFrame& frame_;
    int previous_;
};

/**
 * @brief RAII scope that clips everything submitted inside it to a rect.
 *
 * The clipping counterpart of LayerScope: scopes nest, and a nested
 * clip is intersected with the enclosing one (a scroll view inside a
 * scroll view can never draw outside either), and leaving the scope
 * restores the previous clip.
 */
class ClipScope {
public:
    ClipScope(RenderFrame& frame, const intrnl::Rect& clip);
    ~ClipScope();

    ClipScope(const ClipScope&) = delete;
    ClipScope& operator=(const ClipScope&) = delete;

private:
    RenderFrame& frame_;
    std::optional<intrnl::Rect> previous_;
};
} // namespace sif::rnd

#endif // RENDER_ENGINE_AST_RENDERFRAME_H