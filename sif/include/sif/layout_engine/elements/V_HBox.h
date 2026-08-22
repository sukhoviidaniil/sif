/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-19
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_V_HBOX_H
#define RENDER_ENGINE_V_HBOX_H

#include "LinearLayout.h"

namespace sif::ui {
    /**
     * @brief Vertical box layout.
     *
     * Arranges child elements vertically in a single column.
     * Inherits linear layout behavior with horizontal() = false.
     */
    class VBox : public LinearLayout {
    protected:
        /// @brief Indicates that the layout is vertical.
        bool horizontal() const override { return false; }
    };

    /**
     * @brief Horizontal box layout.
     *
     * Arranges child elements horizontally in a single row.
     * Inherits linear layout behavior with horizontal() = true.
     */
    class HBox : public LinearLayout {
    protected:
        /// @brief Indicates that the layout is horizontal.
        bool horizontal() const override { return true; }
    };
} // namespace sif::ui

#endif // RENDER_ENGINE_V_HBOX_H