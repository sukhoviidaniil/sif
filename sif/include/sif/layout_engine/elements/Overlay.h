/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UI_OVERLAY_H
#define RENDER_ENGINE_UI_OVERLAY_H

#include "UIElement.h"

#include "sif/internal/Rect.h"
#include "sif/math/Vector2.h"

namespace sif::ui {
/**
 * @brief Container whose children overlap instead of being lined up.
 *
 * Every visible child is laid out inside the *same* content box (the
 * Overlay's rect minus padding), so children are drawn one over
 * another - a background with a label on it, a badge in the corner of
 * an icon, a pause panel over the game HUD. It is the one thing a
 * VBox/HBox cannot express: those give every child its own slot.
 *
 * Each child places itself with attached properties it carries (see
 * UIElement): h_align / v_align pick the edge or centre of the box
 * (Stretch fills that axis), margin insets the box for that child, and
 * offset shifts the result afterwards - which is also how a child is
 * positioned absolutely (Start/Start + offset) or made to stick out of
 * its parent (a negative offset). Which child covers which follows the
 * usual paint order: document order, then z_index, then layer.
 *
 * The same idea is FrameLayout on Android, GtkOverlay in GTK and Stack
 * in Flutter; the name follows GTK, which also has VBox/HBox, because
 * "Stack" already means a *linear* layout in WPF (StackPanel) and a
 * one-child-at-a-time switcher in GTK and Qt.
 */
class Overlay : public UIElement {
public:
    ~Overlay() override = default;

    /**
     * @brief Desired size: the largest child (plus its margin), plus padding.
     *
     * An explicit width/height (PX/PERCENT) overrides the content size
     * on that axis; Auto keeps it, like every other container. offset
     * does not count - it moves a child, it does not make room for it.
     *
     * @param available Available space from the parent.
     * @return Desired size of the overlay.
     */
    math::Vector2 measure(const math::Vector2& available) override;

    /**
     * @brief Places every visible child inside the shared content box.
     *
     * @param r Final rectangle assigned by the parent layout.
     */
    void layout(intrnl::Rect r) override;

private:
    /**
     * @brief Position and size of a child along one axis.
     *
     * @param align How the child is aligned on this axis.
     * @param start Start of the (margin-inset) box on this axis.
     * @param extent Length of the box on this axis.
     * @param desired The child's measured length on this axis.
     * @return {position, length}; Stretch returns the whole box, the
     * other alignments keep the desired length (which may overflow
     * the box - overlap is the point of this container, so nothing is
     * clamped or clipped).
     */
    [[nodiscard]] static math::Vector2 place(intrnl::Align align, float start, float extent, float desired);
};
} // namespace sif::ui

#endif // RENDER_ENGINE_UI_OVERLAY_H
