/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-19
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ALIGNMENT_H
#define RENDER_ENGINE_ALIGNMENT_H

namespace sif::intrnl {

    /**
     * @brief Alignment policy used by the layout engine.
     *
     * Defines how an element is positioned or sized along an axis
     * relative to the available layout space.
     */
    enum class Align {
        Start,  ///< Align to the beginning of the available space
        Center, ///< Center within the available space
        End,    ///< Align to the end of the available space
        Stretch ///< Stretch to fill the available space
    };
}

#endif //RENDER_ENGINE_ALIGNMENT_H