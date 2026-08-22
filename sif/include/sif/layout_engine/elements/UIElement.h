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

#include <functional>
#include <memory>
#include <string>

#include "sif/internal/RecordID.h"
#include "sif/internal/Rect.h"
#include "sif/internal/Size.h"
#include "sif/math/Vector2.h"

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
         *
         * Ownership is shared to allow flexible tree composition.
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

        virtual void append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const;

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
        template<typename T = UIElement>
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
        template<typename T = UIElement>
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

        bool visible = true; ///< Visibility flag

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

    template<typename T>
    T* UIElement::find_by_name(const std::string& target_name) {
        UIElement* found = find_if([&](const UIElement& e) { return e.name == target_name; });
        return dynamic_cast<T*>(found);
    }

    template<typename T>
    T* UIElement::find_by_id(const intrnl::RecordID target_id) {
        UIElement* found = find_if([&](const UIElement& e) { return e.id == target_id; });
        return dynamic_cast<T*>(found);
    }
} // namespace sif::ui

#endif // RENDER_ENGINE_UIELEMENT_H