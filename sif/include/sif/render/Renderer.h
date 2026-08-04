/***************************************************************
* Project:          Render_Engine
 * File:            Renderer.h
 *
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2025-12-06
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_VIEW_H
#define RENDER_ENGINE_VIEW_H

#include <memory>

#include "elements/RenderNode.h"

#include "sif/event/Observer.h"
#include "sif/math/Vector2.h"
#include "sif/render/RenderFrame.h"

namespace sif::rnd {
    class Renderer : public RenderVisitor, public event::Observer {
    public:
        ~Renderer() override;
        explicit Renderer();

        virtual void track_local(const std::shared_ptr<event::Event_Bus>& bus) = 0;
        virtual void track_global(const std::shared_ptr<event::Event_Bus>& bus) = 0;

        [[nodiscard]] virtual math::Vector2 screen_size() const = 0;

        virtual void render(const RenderFrame& graph) = 0;
    };
}

#endif //RENDER_ENGINE_VIEW_H