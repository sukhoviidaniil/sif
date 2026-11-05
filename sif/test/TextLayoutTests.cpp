/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-11-05
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"
#include "Fixtures.h"

#include <memory>
#include <string>
#include <vector>

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/Font.h"
#include "sif/layout_engine/elements/Text.h"
#include "sif/render/FrameContext.h"
#include "sif/render/RenderFrame.h"
#include "sif/render/elements/Text.h"

using namespace sif;

namespace {
    /**
     * @brief A font whose metrics are trivial to reason about: every
     * character (UTF-8 code point) is size/2 wide, every line size tall.
     */
    struct GridFont : asset::Font {
        math::Vector2 measure_text(const std::string& str, const unsigned int size) const override {
            std::size_t characters = 0;
            for (const char c : str) {
                if ((static_cast<unsigned char>(c) & 0xC0u) != 0x80u) {
                    ++characters;
                }
            }
            return {static_cast<float>(characters) * static_cast<float>(size) * 0.5f, static_cast<float>(size)};
        }
    };

    /// @brief A ready Font handle backed by GridFont.
    asset::AssetHandle<asset::Font> grid_font() {
        asset::AssetRegistry& registry = asset::AssetRegistry::instance();
        const intrnl::GUID guid = sif::test::fixtures::make_test_asset(registry, asset::AssetType::Font);
        const auto record = registry.get<void>(guid).record().lock();
        if (record != nullptr) {
            const std::shared_ptr<asset::Font> font = std::make_shared<GridFont>();
            record->set_data(std::static_pointer_cast<void>(font));
            record->set_state(asset::AssetState::Ready);
        }
        return registry.get<asset::Font>(guid);
    }

    /// @brief A Text with fontSize 10, so every character is 5 px wide.
    std::unique_ptr<ui::Text> text(const std::string& value) {
        auto t = std::make_unique<ui::Text>(grid_font());
        t->text = value;
        t->fontSize = 10;
        return t;
    }

    std::vector<const rnd::Text*> render(ui::Text& t, const intrnl::Rect rect) {
        t.measure({rect.width, rect.height});
        t.layout(rect);
        static rnd::RenderFrame frame; // keeps the items alive for the caller
        frame = rnd::RenderFrame{};
        const rnd::FrameContext ctx(true);
        t.render(frame, ctx);
        std::vector<const rnd::Text*> items;
        for (const auto& item : frame.temp_items) {
            items.push_back(dynamic_cast<const rnd::Text*>(item.get()));
        }
        return items;
    }

    const std::string k_ellipsis = "\xE2\x80\xA6";
}

SIF_TEST(text_default_is_the_single_top_left_line_it_always_was) {
    auto t = text("hello");
    const auto items = render(*t, {10.f, 20.f, 200.f, 40.f});
    SIF_CHECK(items.size() == 1);
    if (items.size() == 1) {
        SIF_CHECK(items[0]->text == "hello");
        SIF_CHECK(items[0]->rect == intrnl::Rect(10.f, 20.f, 200.f, 40.f));
    }
}

SIF_TEST(text_aligns_inside_its_rect) {
    auto t = text("hello"); // 25 x 10
    t->h_text_align = intrnl::Align::Center;
    t->v_text_align = intrnl::Align::End;
    const auto items = render(*t, {0.f, 0.f, 105.f, 40.f});
    SIF_CHECK(items.size() == 1 && items[0]->rect == intrnl::Rect(40.f, 30.f, 25.f, 10.f));

    t->h_text_align = intrnl::Align::End;
    t->v_text_align = intrnl::Align::Center;
    const auto moved = render(*t, {0.f, 0.f, 105.f, 40.f});
    SIF_CHECK(moved.size() == 1 && moved[0]->rect == intrnl::Rect(80.f, 15.f, 25.f, 10.f));
}

SIF_TEST(text_wraps_words_to_its_width_with_hard_breaks) {
    auto t = text("one two three four\n    five"); // indentation after \n is dropped
    t->wrap = true;
    // 50 px = 10 characters per line.
    t->measure({50.f, 500.f});
    t->layout({0.f, 0.f, 50.f, 500.f});
    SIF_CHECK((t->lines() == std::vector<std::string>{"one two", "three four", "five"}));

    // The wrapped block is what the element asks for.
    const math::Vector2 size = t->measure({50.f, 500.f});
    SIF_CHECK(size.x == 50.f);                  // widest line, "three four"
    SIF_CHECK(size.y == 3.f * t->line_height()); // 3 lines of 12.5 px

    const auto items = render(*t, {0.f, 0.f, 50.f, 500.f});
    SIF_CHECK(items.size() == 3);
    if (items.size() == 3) {
        SIF_CHECK(items[1]->rect.y == t->line_height());
        SIF_CHECK(items[2]->text == "five");
    }

    // A word wider than the line is kept whole rather than split.
    auto long_word = text("supercalifragilistic go");
    long_word->wrap = true;
    long_word->layout({0.f, 0.f, 50.f, 500.f});
    SIF_CHECK((long_word->lines() == std::vector<std::string>{"supercalifragilistic", "go"}));
}

SIF_TEST(text_ellipsis_shortens_a_line_that_does_not_fit) {
    auto t = text("a rather long label");
    t->ellipsis = true;
    t->layout({0.f, 0.f, 50.f, 20.f}); // room for 10 characters
    SIF_CHECK(t->lines().size() == 1 && t->lines()[0] == "a rather" + k_ellipsis);

    t->ellipsis = false; // the cache notices the change
    SIF_CHECK(t->lines()[0] == "a rather long label");
}

SIF_TEST(text_max_lines_and_height_limit_a_wrapped_block) {
    auto t = text("one two three four five six");
    t->wrap = true;
    t->ellipsis = true;
    t->max_lines = 2;
    t->layout({0.f, 0.f, 50.f, 500.f});
    SIF_CHECK((t->lines() == std::vector<std::string>{"one two", "three fou" + k_ellipsis})); // cut per character

    t->max_lines = 0;
    t->layout({0.f, 0.f, 50.f, 26.f}); // 2 lines of 12.5 px fit
    SIF_CHECK(t->lines().size() == 2);
    SIF_CHECK(t->lines().back().ends_with(k_ellipsis));
}

SIF_TEST(text_cyrillic_is_shortened_by_whole_characters) {
    auto t = text("\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD1\x96\xD1\x82, \xD1\x81\xD0\xB2\xD1\x96\xD1\x82\xD0\xB5"); // "Привіт, світе"
    t->ellipsis = true;
    t->layout({0.f, 0.f, 35.f, 20.f}); // 7 characters
    SIF_CHECK(t->lines().size() == 1 && t->lines()[0] == "\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD1\x96\xD1\x82" + k_ellipsis);
}

SIF_TEST(text_honours_an_explicit_size_even_before_its_font_loads) {
    auto t = text("hi");
    t->height = intrnl::Size::px(34.f);
    t->width = intrnl::Size("PERCENT|50");
    const math::Vector2 size = t->measure({400.f, 300.f});
    SIF_CHECK(size.x == 200.f && size.y == 34.f);

    ui::Text loading{asset::AssetHandle<asset::Font>{}};
    loading.text = "not yet";
    loading.height = intrnl::Size::px(34.f);
    SIF_CHECK(loading.measure({400.f, 300.f}).y == 34.f);
}

SIF_TEST(text_changed_after_layout_still_renders_the_current_value) {
    auto t = text("old");
    t->measure({100.f, 20.f});
    t->layout({0.f, 0.f, 100.f, 20.f});
    t->text = "new"; // e.g. a demo's update() writing a label after layout
    rnd::RenderFrame frame;
    t->render(frame, rnd::FrameContext(true));
    SIF_CHECK(frame.temp_items.size() == 1);
    if (frame.temp_items.size() == 1) {
        SIF_CHECK(dynamic_cast<const rnd::Text*>(frame.temp_items[0].get())->text == "new");
    }
}
