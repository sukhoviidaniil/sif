/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-08-03
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"
#include "Fixtures.h"

#include <memory>
#include <type_traits>

#include "sif/asset/AssetRegistry.h"
#include "sif/internal/Delta_Timer.h"
#include "sif/layout_engine/Parser.h"
#include "sif/layout_engine/elements/Animation.h"
#include "sif/layout_engine/elements/Sprite.h"
#include "sif/render/FrameContext.h"
#include "sif/render/RenderFrame.h"
#include "sif/render/elements/Sprite.h"

using namespace sif;
using namespace sif::test::fixtures;

namespace {
    /**
     * @brief Registers an asset, loads it with an immediately-succeeding
     * loader and returns its GUID, so element tests can work against a
     * genuinely Ready record instead of faking readiness.
     */
    intrnl::GUID ready_asset(const asset::AssetType type, std::shared_ptr<void> data) {
        asset::AssetRegistry& registry = asset::AssetRegistry::instance();
        const intrnl::GUID guid = make_test_asset(registry, type);

        // The registry hands out handles to the record; filling the
        // record directly keeps this test free of any real file I/O.
        const asset::AssetHandle<void> handle = registry.get<void>(guid);
        const auto record = handle.record().lock();
        if (record != nullptr) {
            record->set_data(std::move(data));
            record->set_state(asset::AssetState::Ready);
        }
        return guid;
    }

    math::Vector<intrnl::Rect> atlas_rects() {
        math::Vector<intrnl::Rect> rects(3);
        rects[0] = intrnl::Rect(0, 0, 16, 16);
        rects[1] = intrnl::Rect(16, 0, 16, 16);
        rects[2] = intrnl::Rect(32, 0, 16, 16);
        return rects;
    }

    /// @brief Runs one layout + render pass and returns the frame.
    rnd::RenderFrame render_once(ui::UIElement& element, const intrnl::Rect rect) {
        element.measure({rect.width, rect.height});
        element.layout(rect);

        rnd::RenderFrame frame;
        const rnd::FrameContext ctx(true);
        element.append_render_items(frame, ctx);
        return frame;
    }

    const rnd::Sprite* first_sprite(const rnd::RenderFrame& frame) {
        for (const auto& item : frame.temp_items) {
            if (const auto* sprite = dynamic_cast<const rnd::Sprite*>(item.get())) {
                return sprite;
            }
        }
        return nullptr;
    }
}

SIF_TEST(sprite_element_emits_nothing_while_its_asset_is_loading) {
    asset::AssetRegistry& registry = asset::AssetRegistry::instance();
    const intrnl::GUID guid = make_test_asset(registry, asset::AssetType::SpriteSingle);

    ui::Sprite sprite(registry.get<void>(guid), asset::AssetType::SpriteSingle);
    const rnd::RenderFrame frame = render_once(sprite, {0, 0, 64, 64});

    SIF_CHECK(!sprite.ready());
    SIF_CHECK(frame.temp_items.empty());
    SIF_CHECK(frame.constant_items.empty());
}

SIF_TEST(sprite_element_draws_the_whole_texture_for_a_sprite_single) {
    const intrnl::GUID guid = ready_asset(
        asset::AssetType::SpriteSingle,
        std::static_pointer_cast<void>(std::shared_ptr<asset::Sprite>(std::make_shared<FakeSprite>()))
    );

    ui::Sprite sprite(asset::AssetRegistry::instance().get<void>(guid), asset::AssetType::SpriteSingle);
    const rnd::RenderFrame frame = render_once(sprite, {5, 6, 64, 32});

    const rnd::Sprite* item = first_sprite(frame);
    SIF_CHECK(item != nullptr);
    if (item != nullptr) {
        SIF_CHECK(item->kind == asset::AssetType::SpriteSingle);
        SIF_CHECK(item->rect == intrnl::Rect(5, 6, 64, 32));
        // Zero-sized source rect is the agreed encoding of "everything".
        SIF_CHECK(item->src_rect.width == 0.f && item->src_rect.height == 0.f);
    }
}

SIF_TEST(sprite_element_resolves_an_atlas_record_to_its_sub_rect) {
    const intrnl::GUID guid = ready_asset(
        asset::AssetType::SpriteAtlas,
        std::static_pointer_cast<void>(
            std::shared_ptr<asset::SpriteAtlas>(std::make_shared<FakeSpriteAtlas>(atlas_rects())))
    );

    ui::Sprite sprite(asset::AssetRegistry::instance().get<void>(guid),
                      asset::AssetType::SpriteAtlas,
                      intrnl::RecordID(2));
    const rnd::RenderFrame frame = render_once(sprite, {0, 0, 16, 16});

    const rnd::Sprite* item = first_sprite(frame);
    SIF_CHECK(item != nullptr);
    if (item != nullptr) {
        SIF_CHECK(item->src_rect == intrnl::Rect(32, 0, 16, 16));
    }

    // Selecting another record changes the sub-rect, not the asset.
    sprite.set_record_id(intrnl::RecordID(1));
    // The frame owns the render items, so it must outlive the pointer.
    const rnd::RenderFrame second_frame = render_once(sprite, {0, 0, 16, 16});
    const rnd::Sprite* second = first_sprite(second_frame);
    SIF_CHECK(second != nullptr && second->src_rect == intrnl::Rect(16, 0, 16, 16));
}

SIF_TEST(sprite_element_falls_back_to_whole_texture_for_an_out_of_range_record) {
    const intrnl::GUID guid = ready_asset(
        asset::AssetType::SpriteAtlas,
        std::static_pointer_cast<void>(
            std::shared_ptr<asset::SpriteAtlas>(std::make_shared<FakeSpriteAtlas>(atlas_rects())))
    );

    ui::Sprite sprite(asset::AssetRegistry::instance().get<void>(guid),
                      asset::AssetType::SpriteAtlas,
                      intrnl::RecordID(99));

    // A bad id must not throw out of a render pass.
    const rnd::RenderFrame frame = render_once(sprite, {0, 0, 16, 16});
    const rnd::Sprite* item = first_sprite(frame);
    SIF_CHECK(item != nullptr);
    if (item != nullptr) {
        SIF_CHECK(item->src_rect.width == 0.f);
    }
}

SIF_TEST(animation_element_advances_its_own_cursor) {
    math::Vector<intrnl::Rect> frames(4);
    for (size_t i = 0; i < 4; ++i) {
        frames[i] = intrnl::Rect(static_cast<float>(i) * 8.f, 0, 8, 8);
    }

    const intrnl::GUID guid = ready_asset(
        asset::AssetType::PrimitiveAnimation,
        std::static_pointer_cast<void>(
            std::shared_ptr<asset::PrimitiveAnimation>(
                std::make_shared<FakePrimitiveAnimation>(std::move(frames), 0.1f, true)))
    );

    ui::Animation animation(
        asset::AssetRegistry::instance().get<asset::PrimitiveAnimation>(guid));

    SIF_CHECK(animation.current_frame() == 0);

    animation.update(0.25f);
    SIF_CHECK(animation.current_frame() == 2);

    // speed scales the cursor, not the asset.
    animation.speed = 2.f;
    animation.update(0.1f); // +0.2s -> 0.45s
    SIF_CHECK(animation.current_frame() == 0); // wrapped past 0.4s total

    animation.restart();
    SIF_CHECK(animation.current_frame() == 0);
}

SIF_TEST(paused_animation_does_not_advance) {
    math::Vector<intrnl::Rect> frames(2);
    frames[0] = intrnl::Rect(0, 0, 8, 8);
    frames[1] = intrnl::Rect(8, 0, 8, 8);

    const intrnl::GUID guid = ready_asset(
        asset::AssetType::PrimitiveAnimation,
        std::static_pointer_cast<void>(
            std::shared_ptr<asset::PrimitiveAnimation>(
                std::make_shared<FakePrimitiveAnimation>(std::move(frames), 0.1f, true)))
    );

    ui::Animation animation(
        asset::AssetRegistry::instance().get<asset::PrimitiveAnimation>(guid));

    animation.pause();
    animation.update(5.f);
    SIF_CHECK(!animation.playing());
    SIF_CHECK(animation.elapsed_seconds() == 0.f);

    animation.play();
    animation.update(0.15f);
    SIF_CHECK(animation.current_frame() == 1);
}

SIF_TEST(animation_element_emits_the_frame_rect_of_the_current_cursor) {
    math::Vector<intrnl::Rect> frames(3);
    frames[0] = intrnl::Rect(0, 0, 8, 8);
    frames[1] = intrnl::Rect(8, 0, 8, 8);
    frames[2] = intrnl::Rect(16, 0, 8, 8);

    const intrnl::GUID guid = ready_asset(
        asset::AssetType::PrimitiveAnimation,
        std::static_pointer_cast<void>(
            std::shared_ptr<asset::PrimitiveAnimation>(
                std::make_shared<FakePrimitiveAnimation>(std::move(frames), 0.1f, true)))
    );

    ui::Animation animation(
        asset::AssetRegistry::instance().get<asset::PrimitiveAnimation>(guid));
    animation.update(0.15f);

    const rnd::RenderFrame frame = render_once(animation, {0, 0, 32, 32});
    const rnd::Sprite* item = first_sprite(frame);
    SIF_CHECK(item != nullptr);
    if (item != nullptr) {
        SIF_CHECK(item->kind == asset::AssetType::PrimitiveAnimation);
        SIF_CHECK(item->src_rect == intrnl::Rect(8, 0, 8, 8));
    }
}

SIF_TEST(parser_rejects_a_stray_closing_tag_instead_of_popping_an_empty_stack) {
    const std::vector<ui::Token> tokens = {
        ui::Token{ui::TokenType::CloseTag, "VBox", {}}
    };

    bool threw = false;
    try {
        (void)ui::Parser::parse(tokens);
    } catch (const std::exception&) {
        threw = true;
    }
    SIF_CHECK(threw);
}

SIF_TEST(parser_rejects_mismatched_tags) {
    const std::vector<ui::Token> tokens = {
        ui::Token{ui::TokenType::OpenTag, "VBox", {}},
        ui::Token{ui::TokenType::CloseTag, "HBox", {}}
    };

    bool threw = false;
    try {
        (void)ui::Parser::parse(tokens);
    } catch (const std::exception&) {
        threw = true;
    }
    SIF_CHECK(threw);
}

SIF_TEST(registry_reports_an_unknown_guid_instead_of_terminating) {
    bool threw = false;
    try {
        (void)asset::AssetRegistry::instance().get<void>(intrnl::GUID(0xDEADBEEF));
    } catch (const std::exception&) {
        threw = true;
    }
    SIF_CHECK(threw);
}

SIF_TEST(registry_resolves_record_names_declared_in_metadata) {
    asset::AssetRegistry& registry = asset::AssetRegistry::instance();

    asset::data::AssetMetaData meta;
    meta.guid = intrnl::GUID(0x5A11AD);
    meta.type = asset::AssetType::SpriteAtlas;
    meta.asset_name = "named_atlas";
    meta.record_id_to_name = {{0, "head"}, {1, "tail"}};
    meta.record_name_to_id = {{"head", 0}, {"tail", 1}};

    registry.add_AssetRecord(asset::AssetDesc("named_atlas.asset.json", meta));

    SIF_CHECK(registry.type_of(meta.guid) == asset::AssetType::SpriteAtlas);
    SIF_CHECK(registry.record_id_of(meta.guid, "tail") == intrnl::RecordID(1));

    bool threw = false;
    try {
        (void)registry.record_id_of(meta.guid, "nope");
    } catch (const std::exception&) {
        threw = true;
    }
    SIF_CHECK(threw);
}

SIF_TEST(delta_timer_is_a_single_non_copyable_instance) {
    // Same object every time: two "clocks" would mean two different
    // deltaTimes inside one frame.
    SIF_CHECK(&intrnl::Delta_Timer::instance() == &intrnl::Delta_Timer::instance());

    // And the type refuses to be duplicated at all.
    SIF_CHECK(!std::is_copy_constructible_v<intrnl::Delta_Timer>);
    SIF_CHECK(!std::is_move_constructible_v<intrnl::Delta_Timer>);
    SIF_CHECK(!std::is_copy_assignable_v<intrnl::Delta_Timer>);

    // Ticking twice in a row must not go backwards in time.
    intrnl::Delta_Timer::instance().tick();
    SIF_CHECK(intrnl::Delta_Timer::instance().tick() >= 0.f);
}
