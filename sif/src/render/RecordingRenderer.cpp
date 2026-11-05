/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2026-11-05
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#include "sif/render/RecordingRenderer.h"

#include "sif/event/events/window.hpp"
#include "sif/render/elements/Rectangle.h"
#include "sif/render/elements/Sprite.h"
#include "sif/render/elements/Text.h"

namespace sif::rnd {
RecordingRenderer::RecordingRenderer(const math::Vector2 screen_size) : screen_size_(screen_size) {}

RecordingRenderer::~RecordingRenderer() = default;

void RecordingRenderer::track_global(const std::shared_ptr<event::Event_Bus>& bus) {
    using namespace event::window;

    track(bus->subscribe<Window_Closed>([this](const Window_Closed&) { open_ = false; }));
    track(bus->subscribe<Window_Resized>([this](const Window_Resized& e) {
        screen_size_ = {static_cast<float>(e.width), static_cast<float>(e.height)};
    }));
}

void RecordingRenderer::track_local(const std::shared_ptr<event::Event_Bus>& /*bus*/) {}

math::Vector2 RecordingRenderer::screen_size() const { return screen_size_; }

void RecordingRenderer::set_screen_size(const math::Vector2 size) { screen_size_ = size; }

void RecordingRenderer::render(const RenderFrame& graph) {
    frame_.clear();
    // The same traversal a real backend performs (see SFML_Renderer),
    // so what is recorded is exactly what would have been drawn.
    for (const RenderNode* item : graph.draw_order()) {
        item->accept(*this);
    }
    ++frames_rendered_;
}

const std::vector<DrawCall>& RecordingRenderer::last_frame() const { return frame_; }

std::size_t RecordingRenderer::frames_rendered() const { return frames_rendered_; }

bool RecordingRenderer::is_open() const { return open_; }

void RecordingRenderer::visit(const Text& t) {
    DrawCall call;
    call.kind = DrawCall::Kind::Text;
    call.rect = t.rect;
    call.layer = t.layer;
    call.clip = t.clip;
    call.color = t.color;
    call.text = t.text;
    call.text_size = t.size;
    frame_.push_back(std::move(call));
}

void RecordingRenderer::visit(const Rectangle& r) {
    DrawCall call;
    call.kind = DrawCall::Kind::Rectangle;
    call.rect = r.rect;
    call.layer = r.layer;
    call.clip = r.clip;
    call.color = r.color;
    frame_.push_back(std::move(call));
}

void RecordingRenderer::visit(const Sprite& s) {
    DrawCall call;
    call.kind = DrawCall::Kind::Sprite;
    call.rect = s.rect;
    call.layer = s.layer;
    call.clip = s.clip;
    call.color = s.tint;
    call.src_rect = s.src_rect;
    call.asset_kind = s.kind;
    frame_.push_back(std::move(call));
}
} // namespace sif::rnd
