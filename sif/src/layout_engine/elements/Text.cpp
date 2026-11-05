/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/elements/Text.h"
#include "sif/render/elements/Text.h"

#include <algorithm>
#include <cmath>

namespace sif::ui {
namespace {
/// @brief "…" (U+2026) in UTF-8.
constexpr const char* k_ellipsis = "\xE2\x80\xA6";

/// @brief Removes the last UTF-8 character of s (never half of one).
void pop_utf8(std::string& s) {
    while (!s.empty() && (static_cast<unsigned char>(s.back()) & 0xC0u) == 0x80u) {
        s.pop_back();
    }
    if (!s.empty()) {
        s.pop_back();
    }
}

bool is_blank(const char c) { return c == ' ' || c == '\t' || c == '\r'; }

/// @brief Offset of something `length` long inside `extent`, for an alignment.
float align_offset(const intrnl::Align align, const float extent, const float length) {
    switch (align) {
    case intrnl::Align::Center:
        return (extent - length) * 0.5f;
    case intrnl::Align::End:
        return extent - length;
    case intrnl::Align::Start:
    case intrnl::Align::Stretch:
    default:
        return 0.f;
    }
}
} // namespace

float Text::width_of(const std::string& line, const unsigned int size) const {
    const asset::Font* f = font.get();
    return f == nullptr || line.empty() ? 0.f : f->measure_text(line, size).x;
}

float Text::line_height() const { return static_cast<float>(std::max(0, fontSize)) * line_spacing; }

std::vector<std::string> Text::wrap_lines(const float max_width, const unsigned int size) const {
    std::vector<std::string> out;

    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t end = std::min(text.find('\n', start), text.size());
        const std::string_view paragraph(text.data() + start, end - start);

        std::string current;
        std::size_t i = 0;
        while (i < paragraph.size()) {
            // Runs of spaces/tabs separate words (and indentation after
            // a '\n' is dropped with them).
            while (i < paragraph.size() && is_blank(paragraph[i])) {
                ++i;
            }
            const std::size_t word_start = i;
            while (i < paragraph.size() && !is_blank(paragraph[i])) {
                ++i;
            }
            if (word_start == i) {
                break;
            }
            const std::string word(paragraph.substr(word_start, i - word_start));

            std::string candidate = current.empty() ? word : current + " " + word;
            if (current.empty() || width_of(candidate, size) <= max_width) {
                current = std::move(candidate);
            } else {
                out.push_back(std::move(current));
                current = word;
            }
        }
        out.push_back(std::move(current));

        if (end >= text.size()) {
            break;
        }
        start = end + 1;
    }
    return out;
}

std::string Text::ellipsize(std::string line, const float max_width, const unsigned int size) const {
    std::string shortened = line + k_ellipsis;
    while (!line.empty() && width_of(shortened, size) > max_width) {
        pop_utf8(line);
        while (!line.empty() && is_blank(line.back())) {
            line.pop_back(); // "word …" reads worse than "word…"
        }
        shortened = line + k_ellipsis;
    }
    return shortened;
}

std::vector<std::string> Text::compose(const float max_width, const float max_height) const {
    const auto size = static_cast<unsigned int>(std::max(0, fontSize));

    if (!wrap) {
        // One line, exactly as before - unless it is too wide and may
        // be shortened.
        if (ellipsis && width_of(text, size) > max_width) {
            return {ellipsize(text, max_width, size)};
        }
        return {text};
    }

    std::vector<std::string> lines = wrap_lines(max_width, size);

    std::size_t keep = lines.size();
    if (max_lines > 0) {
        keep = std::min(keep, static_cast<std::size_t>(max_lines));
    }
    if (ellipsis && line_height() > 0.f) {
        // Only as many lines as the rect is tall (at least one).
        const auto fit = static_cast<std::size_t>(std::max(1.f, std::floor(max_height / line_height())));
        keep = std::min(keep, fit);
    }

    if (keep < lines.size()) {
        lines.resize(keep);
        if (ellipsis && !lines.empty()) {
            lines.back() = ellipsize(lines.back(), max_width, size);
        }
    } else if (ellipsis) {
        // Nothing dropped, but a single over-long word still has to fit.
        for (std::string& line : lines) {
            if (width_of(line, size) > max_width) {
                line = ellipsize(line, max_width, size);
            }
        }
    }
    return lines;
}

math::Vector2 Text::measure(const math::Vector2& available) {
    math::Vector2 measured{0.f, 0.f};

    if (font.ready() && !text.empty()) {
        const auto size = static_cast<unsigned int>(std::max(0, fontSize));

        if (wrap) {
            // Wrap to the width this text will actually get.
            const float wrap_width = width.type != intrnl::Size::Type::Auto ? width.resolve(available.x) : available.x;
            const float wrap_height =
                height.type != intrnl::Size::Type::Auto ? height.resolve(available.y) : available.y;
            const std::vector<std::string> block = compose(wrap_width, wrap_height);
            for (const std::string& line : block) {
                measured.x = std::max(measured.x, width_of(line, size));
            }
            measured.y = static_cast<float>(block.size()) * line_height();
        } else {
            // Ask the font for real metrics instead of guessing
            // "characters * font size": the estimate was wrong for every
            // proportional font, which is most of them.
            measured = font.get()->measure_text(text, size);

            if (stretch_font) {
                measured.x *= 1.1f;
            }
            measured.y = std::max(measured.y, static_cast<float>(size));
        }

        // Never claim more room than the parent offered.
        measured.x = std::min(measured.x, available.x);
        measured.y = std::min(measured.y, available.y);
    }

    // An explicit size wins on its axis. It used to be ignored, so a
    // label declared height="PX|34" got whatever its glyphs measured.
    if (width.type != intrnl::Size::Type::Auto)
        measured.x = width.resolve(available.x);
    if (height.type != intrnl::Size::Type::Auto)
        measured.y = height.resolve(available.y);

    measured.x = std::clamp(measured.x, min_size.x, max_size.x);
    measured.y = std::clamp(measured.y, min_size.y, max_size.y);

    return measured;
}

void Text::layout(const intrnl::Rect r) {
    UIElement::layout(r);
    refresh_lines();
}

void Text::refresh_lines() const {
    const LinesKey key{text,     result.rect.width, result.rect.height, fontSize,    wrap,
                       ellipsis, max_lines,         line_spacing,       font.ready()};
    if (key == lines_key_) {
        return;
    }
    lines_key_ = key;

    lines_.clear();
    line_widths_.clear();
    single_line_height_ = 0.f;

    if (!font.ready() || text.empty()) {
        return;
    }

    const auto size = static_cast<unsigned int>(std::max(0, fontSize));
    lines_ = compose(result.rect.width, result.rect.height);
    for (const std::string& line : lines_) {
        line_widths_.push_back(width_of(line, size));
    }
    if (!wrap) {
        single_line_height_ = std::max(font.get()->measure_text(lines_.front(), size).y, static_cast<float>(size));
    }
}

const std::vector<std::string>& Text::lines() const {
    refresh_lines();
    return lines_;
}

void Text::append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& /*ctx*/) const {
    // visible was the one flag Text never checked; render() now
    // enforces it for every element, and this keeps a direct call to
    // append_render_items consistent with the other leaf elements.
    if (!visible || !font.ready() || text.empty())
        return;

    refresh_lines();
    if (lines_.empty())
        return;

    const intrnl::Rect& r = result.rect;
    const bool block = wrap;
    const float step = block ? line_height() : single_line_height_;
    const float block_height = step * static_cast<float>(lines_.size());
    const float top = r.y + align_offset(v_text_align, r.height, block_height);

    for (std::size_t i = 0; i < lines_.size(); ++i) {
        std::unique_ptr<rnd::Text> item = std::make_unique<rnd::Text>();

        const bool at_origin = !block &&
                               (h_text_align == intrnl::Align::Start || h_text_align == intrnl::Align::Stretch) &&
                               (v_text_align == intrnl::Align::Start || v_text_align == intrnl::Align::Stretch);
        if (at_origin) {
            // The historical single-line case keeps its exact rect.
            item->rect = r;
        } else {
            item->rect = {r.x + align_offset(h_text_align, r.width, line_widths_[i]),
                          top + step * static_cast<float>(i), line_widths_[i], step};
        }
        item->font = font;
        item->text = lines_[i];
        item->size = fontSize;
        item->color = color;
        // submit(), not constant_items: the two buckets are a *caching*
        // distinction (rebuilt every frame vs. not), not a z-order. Using
        // them as one meant every animated entity was painted on top of
        // every label - a pause overlay ended up underneath the bombs it
        // was supposed to cover. Covering is now expressed explicitly:
        // submit() stamps the layer of the enclosing LayerScope, and
        // within a layer draw order is submission order (see
        // RenderFrame::draw_order).
        frame.submit(std::move(item));
    }
}
} // namespace sif::ui
