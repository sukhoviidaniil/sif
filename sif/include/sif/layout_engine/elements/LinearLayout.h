/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-19
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_LINEARLAYOUT_H
#define RENDER_ENGINE_LINEARLAYOUT_H

#include "sif/math/Vector2.h"
#include "sif/internal/Alignment.h"
#include "sif/internal/Rect.h"
#include "sif/layout_engine/elements/UIElement.h"

namespace sif::ui {

    /**
     * @brief Base class for linear layout containers.
     *
     * Arranges child elements in a single row or column, optionally
     * supporting alignment, spacing, and flexible growth.
     */
    class LinearLayout : public UIElement {
    public:
        ~LinearLayout() override = default;

        /// @brief Alignment of children along the cross axis.
        intrnl::Align align = intrnl::Align::Start;
        /// @brief Spacing between consecutive children.
        float spacing = 0.f;

    protected:
        /**
         * @brief Determines if the layout is horizontal or vertical.
         *
         * Must be implemented by derived classes.
         *
         * @return true if horizontal, false if vertical.
         */
        [[nodiscard]] virtual bool horizontal() const = 0;

        /**
         * @brief Measures the desired size of the layout.
         *
         * Accumulates the sizes of visible children along the main axis
         * and takes the maximum along the cross axis. Spacing between
         * children is included in the measurement.
         *
         * @param available Available space from the parent.
         * @return Desired size of the layout.
         */
        math::Vector2 measure(const math::Vector2 &available) override;

        /**
         * @brief Lays out children within the assigned rectangle.
         *
         * Calculates positions and sizes for each child, taking into
         * account fixed and flexible elements, alignment, and spacing.
         *
         * @param r Final rectangle assigned by the parent layout.
         */
        void layout(intrnl::Rect r) override;
    };
}

#endif //RENDER_ENGINE_LINEARLAYOUT_H