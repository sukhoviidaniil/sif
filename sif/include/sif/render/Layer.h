/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2026-11-05
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_LAYER_H
#define RENDER_ENGINE_LAYER_H

#include <string_view>

namespace sif::rnd {
/**
 * @brief Named paint layers.
 *
 * A layer is a plain int: items on a higher layer are drawn over
 * items on a lower one, whatever order they were submitted in (see
 * RenderFrame::draw_order). Within one layer, submission order - i.e.
 * the layout tree's paint order - still decides.
 *
 * The values are deliberately spaced by 100 rather than numbered
 * 0, 1, 2...: a scene that needs "just above Popup but below Modal"
 * writes layer="150" instead of renumbering every named layer. The
 * names are only a vocabulary for the common cases, not a closed set.
 */
namespace layer {
inline constexpr int Background = -100; ///< Wallpaper, backdrops - below normal content
inline constexpr int Content = 0;       ///< Default: everything that does not ask for a layer
inline constexpr int Popup = 100;       ///< HUD panels, dropdowns, badges that must clear the content
inline constexpr int Modal = 200;       ///< Dialogs and pause screens that cover everything below
inline constexpr int Tooltip = 300;     ///< Always on top, even of a modal dialog
} // namespace layer

/**
 * @brief Parses a layer from its name or from a plain integer.
 *
 * Accepts "Background" | "Content" | "Popup" | "Modal" | "Tooltip"
 * (case-sensitive, matching how intrnl::Align values are spelled in
 * UI XML) or any integer such as "150" or "-20".
 *
 * @throws std::invalid_argument if str is neither.
 */
int layer_from_string(std::string_view str);
} // namespace sif::rnd

#endif // RENDER_ENGINE_LAYER_H
