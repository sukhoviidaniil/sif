/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-19
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UIELEMENT_H
#define RENDER_ENGINE_UIELEMENT_H

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "sif/internal/Alignment.h"
#include "sif/internal/RecordID.h"
#include "sif/internal/Rect.h"
#include "sif/internal/Size.h"
#include "sif/math/Vector2.h"

#include "sif/layout_engine/UIEvent.h"
#include "sif/layout_engine/internal/LayoutResult.h"
#include "sif/render/FrameContext.h"
#include "sif/render/RenderFrame.h"

namespace sif::ui {
/**
 * @brief Base class for all UI layout elements.
 *
 * Represents a node in the layout tree. Each UIElement participates
 * in the measure and layout passes and may contain child elements.
 * The class provides default size resolution and recursive layout
 * propagation.
 */
class UIElement {
public:
    /// @brief Virtual destructor.
    virtual ~UIElement() = default;

    /**
     * @brief Adds a child element to this node.
     * Ownership of the child is transferred to this node, allowing
     * the UI tree to manage the lifetime of its child elements.
     * Virtual so that containers with extra bookkeeping needs (e.g.
     * ui::Menu, which tracks Button children separately for
     * set_active) can hook into it and still work correctly when
     * called through a UIElement* / UIFactory::build, which only
     * ever sees the base type.
     */
    virtual void add(std::unique_ptr<UIElement> child);

    // ===== Layout API =====

    /**
     * @brief Measures the desired size of the element.
     *
     * Called during the measure pass with the available space
     * provided by the parent. The default implementation resolves
     * the size from width/height constraints.
     *
     * @param available Available space for this element.
     * @return Desired size of the element.
     */
    virtual math::Vector2 measure(const math::Vector2& available);

    /**
     * @brief Assigns the final layout rectangle to the element.
     *
     * Stores the computed rectangle and propagates the same layout
     * to all child elements by default.
     *
     * @param r Final rectangle assigned by the layout engine.
     */
    virtual void layout(intrnl::Rect r);

    // ===== Render API =====

    /**
     * @brief Submits this element and its subtree to the frame.
     *
     * The entry point for drawing a tree (App calls it on the root,
     * containers call it on each child). Template Method / non-virtual
     * interface: the rules every element must obey - an invisible
     * element hides its whole subtree, an element with a layer moves
     * its whole subtree onto that layer, and a clipping element cuts
     * its whole subtree at its rect - are enforced here,
     * exactly once, and the per-type drawing is left to the virtual
     * append_render_items. Before this existed, visibility was checked
     * (or forgotten - see ui::Text) separately in every element, and
     * hiding a container hid nothing.
     *
     * @param frame Frame being collected.
     * @param ctx Per-frame context passed through to every element.
     */
    void render(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const;

    /**
     * @brief Emits this element's own render items, then its children's.
     *
     * Override to draw something; call the base implementation to
     * keep drawing children. The base implementation submits children
     * through render() in paint_order(), so z_index and layer are
     * honoured for every container without each one re-implementing
     * them. Own items are emitted before children's (pre-order) by
     * every built-in element; element_at relies on that to agree with
     * what is on screen.
     *
     * Calling this directly (instead of render()) skips this element's
     * own visible/layer handling - useful in tests that inspect exactly
     * what one element emits.
     */
    virtual void append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const;

    /**
     * @brief Indices into children, in the order they are painted.
     *
     * Stable by z_index: a child with a higher z_index is painted
     * later (on top of) its siblings, and siblings with equal z_index
     * keep document order - so a scene that never sets z_index paints
     * exactly as before. Ordering is local to one parent (a stacking
     * context, in CSS terms): z_index can lift a child above its
     * siblings, but not out of its parent's place among *its*
     * siblings; for that, see layer.
     */
    [[nodiscard]] std::vector<std::size_t> paint_order() const;

    // ===== Hit testing =====

    /**
     * @brief Path from this element to the topmost element under point.
     *
     * "Topmost" means what is actually drawn on top at that point:
     * the highest effective layer wins, and within a layer the element
     * painted last (see paint_order) wins. Invisible subtrees are
     * skipped; an element with hit_testable == false is skipped itself
     * but its children are still considered. Children are tested even
     * outside their parent's rect, because nothing is clipped (an
     * Overlay child moved out by offset is still drawn, so it is still
     * clickable).
     *
     * Layers are compared relative to this element: it is treated as
     * if its parent were on rnd::layer::Content. That is exact when
     * called on the root, and keeps relative order correct for any
     * subtree.
     *
     * Uses result.rect, so it is only meaningful after layout().
     *
     * @return The ancestor chain, this element first and the topmost
     * hit last; empty if nothing is under point.
     */
    [[nodiscard]] std::vector<UIElement*> hit_path(const math::Vector2& point);

    /**
     * @brief Finds what a click at point lands on.
     *
     * Returns the deepest element of type T on hit_path(point). For
     * T = UIElement that is the topmost element itself; for, say,
     * T = Button it is the button whose label (a child Text) was
     * clicked. Crucially it does *not* look past the topmost hit: if a
     * modal panel covers a button, element_at<Button> returns nullptr
     * rather than the button underneath, which is what makes an
     * overlay actually block input to what it covers.
     *
     * @tparam T Expected concrete element type (defaults to UIElement).
     * @param point Point in layout (screen) coordinates.
     * @return Matching element cast to T*, or nullptr.
     */
    template <typename T = UIElement>
    T* element_at(const math::Vector2& point);

    // ===== Input =====

    /**
     * @brief Receives a UIEvent on its way up the tree (see InputRouter).
     *
     * The default forwards to event_handler, if set. Override to give an
     * element type built-in behaviour (Button clicks, ScrollView wheel),
     * and call the base implementation to keep event_handler working.
     *
     * @return true to stop the event from reaching this element's ancestors.
     */
    virtual bool on_ui_event(UIEvent& e);

    /**
     * @brief Per-instance handler, for reacting to events without subclassing.
     *
     * Same contract as on_ui_event: set e.handled when acting on the
     * event, return true to stop it bubbling further.
     */
    std::function<bool(UIEvent&)> event_handler;

    /**
     * @brief Whether keyboard focus may land here right now.
     *
     * focusable, by default; Button also requires being enabled.
     */
    [[nodiscard]] virtual bool can_focus() const;

    /**
     * @brief Advances any time-based state (animations, transitions).
     *
     * Called once per frame with the elapsed time since the last
     * call. The default implementation simply recurses into every
     * child, so elements without their own time-based behavior
     * (most of them) never need to override this. Elements that do
     * animate something - e.g. ui::Button's selected/unselected
     * blend, or a future primitive-animation UI element - override
     * this to advance their own timer before calling the base
     * implementation for their children.
     *
     * @param dt Elapsed time since the previous update, in seconds.
     */
    virtual void update(float dt);

    // ===== Tree lookup =====

    /**
     * @brief Finds the first descendant (or self) with a matching name.
     *
     * Searches this element and its subtree depth-first. The
     * template parameter lets callers get back a properly typed
     * pointer without a separate cast: find_by_name<Button>("ok")
     * returns Button* directly (or nullptr if no element named
     * "ok" exists, or it exists but is not a Button).
     *
     * @tparam T Expected concrete element type (defaults to UIElement).
     * @param target_name Name to search for (see UIElement::name).
     * @return Matching element cast to T*, or nullptr if not found.
     */
    template <typename T = UIElement>
    T* find_by_name(const std::string& target_name);

    /**
     * @brief Finds the first descendant (or self) with a matching id.
     *
     * Same semantics as find_by_name, but matches on UIElement::id.
     *
     * @tparam T Expected concrete element type (defaults to UIElement).
     * @param target_id Id to search for (see UIElement::id).
     * @return Matching element cast to T*, or nullptr if not found.
     */
    template <typename T = UIElement>
    T* find_by_id(intrnl::RecordID target_id);

    // ===== Configuration =====

    /**
     * @brief Optional human-readable name (e.g. from the "name" XML
     * attribute), used by find_by_name. Empty means unnamed.
     */
    std::string name;

    /**
     * @brief Optional stable id (e.g. from the "id" XML attribute),
     * used by find_by_id. Defaults to id 0, which is treated as
     * "no id assigned" by find_by_id (it never matches).
     */
    intrnl::RecordID id;

    intrnl::Size width = intrnl::Size::auto_();  ///< Width specification
    intrnl::Size height = intrnl::Size::auto_(); ///< Height specification

    math::Vector2 min_size{0, 0};       ///< Minimum allowed size
    math::Vector2 max_size{1e9f, 1e9f}; ///< Maximum allowed size

    math::Vector2 margin{0, 0};  ///< External spacing
    math::Vector2 padding{0, 0}; ///< Internal spacing

    float flex = 0.f; ///< Flex grow factor (0 = fixed, >0 = expandable)

    bool visible = true; ///< Visibility flag (false hides the whole subtree)

    // ===== Layering =====

    /**
     * @brief Paint order among siblings (higher = drawn later, on top).
     *
     * Local to the parent - see paint_order(). 0 for everything by
     * default, which keeps document order.
     */
    int z_index = 0;

    /**
     * @brief Global paint layer for this element and its subtree.
     *
     * nullopt (the default) inherits the parent's layer. Unlike
     * z_index this is not confined to the parent: an element on
     * rnd::layer::Modal is drawn over everything on lower layers,
     * wherever in the tree it is declared - which is what a dialog or
     * a tooltip declared deep inside some panel needs.
     */
    std::optional<int> layer;

    /**
     * @brief Whether element_at/hit_path may return this element.
     *
     * false makes the element itself transparent to clicks (its
     * children stay hit-testable), e.g. a full-screen HUD container
     * that must not swallow clicks meant for the game UI below it.
     */
    bool hit_testable = true;

    /**
     * @brief Clip this element's subtree to its own rect.
     *
     * Everything the subtree draws is cut at result.rect (nested clips
     * intersect), and nothing outside it can be clicked - a child
     * scrolled out of a ScrollView is neither visible nor hittable.
     */
    bool clip = false;

    /// @brief Whether keyboard focus can land on this element (Button: true).
    bool focusable = false;

    // ===== Interaction state (maintained by InputRouter) =====

    bool hovered = false; ///< The pointer is over this element or one of its descendants
    bool pressed = false; ///< A press started on this element (or a descendant) and is not released yet
    bool focused = false; ///< This element has keyboard focus

    // ===== Placement inside a parent Overlay =====
    //
    // Attached properties: they live on the child but are read only
    // by the parent ui::Overlay, the same way flex lives on every
    // element but only a LinearLayout interprets it. Any other
    // container ignores them.

    intrnl::Align h_align = intrnl::Align::Start; ///< Horizontal placement inside an Overlay
    intrnl::Align v_align = intrnl::Align::Start; ///< Vertical placement inside an Overlay
    math::Vector2 offset{0, 0};                   ///< Shift applied after alignment inside an Overlay

    /**
     * @brief Position and size in world units, inside a parent WorldView.
     *
     * Attached property read only by ui::WorldView, which projects it
     * through its rnd::Camera every layout; nullopt fills the whole view.
     */
    std::optional<intrnl::Rect> world_rect;

    LayoutResult result{}; ///< Result of the layout pass

    /// @brief Child elements in the layout tree.
    std::vector<std::unique_ptr<UIElement>> children;

protected:
    /**
     * @brief Resolves the final size based on available space.
     *
     * Converts width and height Size specifications into concrete
     * values and clamps them to the configured min/max constraints.
     *
     * @param available Available space from the parent.
     * @return Resolved size.
     */
    [[nodiscard]] math::Vector2 resolve_size(const math::Vector2& available) const;

private:
    /**
     * @brief Untyped depth-first search shared by find_by_name/find_by_id.
     *
     * Kept out of the header's template bodies so the traversal
     * logic exists exactly once regardless of how many different
     * T's find_by_name<T>/find_by_id<T> get instantiated for.
     */
    UIElement* find_if(const std::function<bool(const UIElement&)>& predicate);
};

template <typename T>
T* UIElement::find_by_name(const std::string& target_name) {
    UIElement* found = find_if([&](const UIElement& e) { return e.name == target_name; });
    return dynamic_cast<T*>(found);
}

template <typename T>
T* UIElement::find_by_id(const intrnl::RecordID target_id) {
    UIElement* found = find_if([&](const UIElement& e) { return e.id == target_id; });
    return dynamic_cast<T*>(found);
}

template <typename T>
T* UIElement::element_at(const math::Vector2& point) {
    const std::vector<UIElement*> path = hit_path(point);
    for (auto it = path.rbegin(); it != path.rend(); ++it) {
        if (T* typed = dynamic_cast<T*>(*it)) {
            return typed;
        }
    }
    return nullptr;
}
} // namespace sif::ui

#endif // RENDER_ENGINE_UIELEMENT_H