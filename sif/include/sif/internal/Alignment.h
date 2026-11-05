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

/**
 * @brief How a linear layout distributes leftover main-axis space.
 *
 * Align positions children across the axis they are stacked along;
 * Justify decides where the space left over *along* that axis goes.
 * It only has an effect when there is leftover space, i.e. when no
 * child has flex (flex children already take all of it).
 */
enum class Justify {
    Start,        ///< Pack children at the start (the historical behaviour)
    Center,       ///< Pack children in the middle
    End,          ///< Pack children at the end
    SpaceBetween, ///< First and last child touch the edges, equal gaps in between
    SpaceAround,  ///< Equal space around every child (half a gap at each edge)
    SpaceEvenly   ///< Equal gaps between children and at both edges
};
} // namespace sif::intrnl

#endif // RENDER_ENGINE_ALIGNMENT_H