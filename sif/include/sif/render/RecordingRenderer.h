/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2026-11-05
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_RECORDINGRENDERER_H
#define RENDER_ENGINE_RECORDINGRENDERER_H

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "sif/asset/internal/AssetType.h"
#include "sif/internal/Color.h"
#include "sif/internal/Rect.h"
#include "sif/render/Renderer.h"

namespace sif::rnd {
/**
 * @brief One draw call as a renderer would have issued it.
 *
 * Plain data copied out of the render item, so a recorded frame stays
 * valid after the RenderFrame it came from is gone.
 */
struct DrawCall {
    enum class Kind { Text, Rectangle, Sprite };

    Kind kind = Kind::Rectangle;
    intrnl::Rect rect{};
    int layer = 0;

    /// Clip rect the item carried (nullopt = not clipped).
    std::optional<intrnl::Rect> clip;

    /// Fill colour (Rectangle), text colour (Text) or tint (Sprite).
    intrnl::Color color{};

    std::string text;  ///< Text only
    int text_size = 0; ///< Text only

    intrnl::Rect src_rect{};                                   ///< Sprite only
    asset::AssetType asset_kind = asset::AssetType::Undefined; ///< Sprite only
};

/**
 * @brief A Renderer that draws nothing and remembers everything.
 *
 * The render backend for RB_Type::Headless, and a Test Spy for the
 * engine's own tests: it walks RenderFrame::draw_order() exactly like a
 * real backend and records each visit() as a DrawCall, so "what would be
 * on screen, in which order, on which layer" becomes something a test or
 * a CI run can assert on - without a window, a GPU or SFML. It lives in
 * the engine (sif_render), not in a backend, because it needs nothing
 * but the engine's own interfaces.
 */
class RecordingRenderer final : public Renderer {
public:
    explicit RecordingRenderer(math::Vector2 screen_size = {800.f, 600.f});
    ~RecordingRenderer() override;

    /// @brief Window_Resized changes screen_size(), Window_Closed sets is_open() to false.
    void track_global(const std::shared_ptr<event::Event_Bus>& bus) override;
    void track_local(const std::shared_ptr<event::Event_Bus>& bus) override;

    [[nodiscard]] math::Vector2 screen_size() const override;
    void set_screen_size(math::Vector2 size);

    /// @brief Records the frame's draw calls, replacing the previous frame's.
    void render(const RenderFrame& graph) override;

    /// @brief Draw calls of the most recent render(), in draw order.
    [[nodiscard]] const std::vector<DrawCall>& last_frame() const;

    /// @brief How many frames have been rendered.
    [[nodiscard]] std::size_t frames_rendered() const;

    [[nodiscard]] bool is_open() const;

private:
    void visit(const Text& t) override;
    void visit(const Rectangle& r) override;
    void visit(const Sprite& s) override;

    math::Vector2 screen_size_;
    std::vector<DrawCall> frame_;
    std::size_t frames_rendered_ = 0;
    bool open_ = true;
};
} // namespace sif::rnd

#endif // RENDER_ENGINE_RECORDINGRENDERER_H
