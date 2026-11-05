/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2025-12-26
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_TEXT_H
#define RENDER_ENGINE_TEXT_H
#include <string>
#include <vector>

#include "UIElement.h"
#include "sif/asset/AssetHandle.h"
#include "sif/asset/internal/Font.h"
#include "sif/internal/Color.h"

namespace sif::ui {
/**
 * @brief Text label: one line, or a block of word-wrapped lines.
 *
 * By default a Text is the single line it always was, drawn at the
 * top-left of its rect. On top of that it can align itself inside its
 * rect (h_text_align / v_text_align), wrap words to its width (wrap,
 * with '\n' as a hard break), cap the number of lines (max_lines) and
 * shorten whatever does not fit with an ellipsis (ellipsis).
 *
 * Line breaking and alignment happen here, in the engine, against
 * asset::Font::measure_text - the backend only ever receives one
 * rnd::Text per line at its final position, so every backend lays text
 * out identically.
 */
class Text : public UIElement {
public:
    /// @brief Virtual destructor.
    ~Text() override = default;
    explicit Text(asset::AssetHandle<asset::Font> f) : font(std::move(f)) {}

    /**
     * @brief Measures the desired size of the text.
     *
     * The natural size comes from the font's real metrics (the whole
     * wrapped block when wrap is on) and never exceeds what the parent
     * offered. An explicit width/height (PX/PERCENT) wins on its axis,
     * as for every other element - and is reported even while the font
     * is still loading, so a label's slot does not jump when it
     * arrives.
     *
     * @param available Available space.
     * @return Desired size of the text.
     */
    math::Vector2 measure(const math::Vector2& available) override;

    /**
     * @brief Breaks the text into lines for the assigned rect.
     */
    void layout(intrnl::Rect r) override;

    void append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const override;

    /// @brief The lines produced by the last layout() (after wrapping, max_lines and ellipsis).
    [[nodiscard]] const std::vector<std::string>& lines() const;

    /// @brief Height of one line of a wrapped block: fontSize * line_spacing.
    [[nodiscard]] float line_height() const;

    std::string text;

    intrnl::Color color = intrnl::Color(255, 255, 255);

    asset::AssetHandle<asset::Font> font;

    int fontSize = 14;

    /// @brief Adds 10% to the measured width (kept as it always behaved).
    bool stretch_font = false;

    /// @brief Horizontal placement of each line inside the rect (Stretch acts as Start).
    intrnl::Align h_text_align = intrnl::Align::Start;

    /// @brief Vertical placement of the line or block inside the rect (Stretch acts as Start).
    intrnl::Align v_text_align = intrnl::Align::Start;

    /**
     * @brief Word-wrap to the rect's width.
     *
     * '\n' is a hard break, and spaces/tabs after it are dropped, so
     * text written over several lines of a scene file keeps those lines
     * instead of picking up the file's indentation. A single word wider
     * than the rect is kept whole (and overflows) rather than split.
     */
    bool wrap = false;

    /// @brief Shorten text that does not fit (width, max_lines or height) and end it with "…".
    bool ellipsis = false;

    /// @brief Most lines a wrapped block may have; 0 = unlimited.
    int max_lines = 0;

    /// @brief Line height of a wrapped block, as a multiple of fontSize.
    float line_spacing = 1.25f;

private:
    /// @brief Greedy word wrap of text into lines no wider than max_width.
    [[nodiscard]] std::vector<std::string> wrap_lines(float max_width, unsigned int size) const;

    /// @brief Shortens line until line + "…" fits max_width.
    [[nodiscard]] std::string ellipsize(std::string line, float max_width, unsigned int size) const;

    /// @brief Lines for a block max_width wide and max_height tall (applies max_lines and ellipsis).
    [[nodiscard]] std::vector<std::string> compose(float max_width, float max_height) const;

    [[nodiscard]] float width_of(const std::string& line, unsigned int size) const;

    /**
     * @brief Recomputes the cached lines if anything they depend on changed.
     *
     * A lazy cache rather than work done only in layout(): game code
     * may change text (or any option) after the layout pass and before
     * rendering, and a label must still show its current text that very
     * frame, as it always did.
     */
    void refresh_lines() const;

    /// @brief Everything the cached lines depend on.
    struct LinesKey {
        std::string text;
        float width = -1.f;
        float height = -1.f;
        int font_size = 0;
        bool wrap = false;
        bool ellipsis = false;
        int max_lines = 0;
        float line_spacing = 0.f;
        bool font_ready = false;

        bool operator==(const LinesKey&) const = default;
    };

    mutable LinesKey lines_key_;
    mutable std::vector<std::string> lines_;
    mutable std::vector<float> line_widths_;
    mutable float single_line_height_ = 0.f;
};
} // namespace sif::ui

#endif // RENDER_ENGINE_TEXT_H