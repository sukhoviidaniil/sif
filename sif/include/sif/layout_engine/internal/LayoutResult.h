/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-19
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_LAYOUTRESULT_H
#define RENDER_ENGINE_LAYOUTRESULT_H

#include "sif/internal/Rect.h"

namespace sif::ui {
    /**
     * @brief Result of a layout computation.
     *
     * Contains the resolved rectangle assigned to a UI element
     * after the layout engine finishes its calculations.
     */
    struct LayoutResult {
        LayoutResult() = default;
        intrnl::Rect rect; ///< Final computed rectangle
    };
}


#endif //RENDER_ENGINE_LAYOUTRESULT_H