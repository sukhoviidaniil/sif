/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-02-25
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "App.h"

#include <stdexcept>
#include <utility>

#include "Graphics_Factory.h"

#include "sif/asset/AssetRegistry.h"
#include "sif/diagnostics/Logger.h"
#include "sif/event/events/input_Keyboard.h"
#include "sif/event/events/window.hpp"
#include "sif/internal/Delta_Timer.h"
#include "sif/render/FrameContext.h"
#include "sif/render/RenderFrame.h"

namespace app {
    using namespace sif;

    App::App(std::shared_ptr<event::Event_Bus> bus,
             const ast::RB_Config &render_config,
             const ast::EC_Config &collector_config,
             std::unique_ptr<demo::Demo> demo)
        : bus_(std::move(bus))
        , demo_(std::move(demo)) {

        if (bus_ == nullptr) {
            throw std::invalid_argument("App: the global event bus must not be null");
        }
        if (demo_ == nullptr) {
            throw std::invalid_argument("App: a demo must be supplied");
        }

        // Closing the window is the one event App itself reacts to;
        // everything else is somebody else's business.
        track(
            bus_->subscribe<event::window::Window_Closed>(
                [this](const event::window::Window_Closed&) {
                    running_ = false;
                }
            )
        );

        Graphics_Factory& factory = Graphics_Factory::instance();

        renderer_ = factory.make_Renderer(render_config);
        renderer_->track_global(bus_);

        collector_ = factory.make_Event_Collector(collector_config);
        audio_ = factory.make_AudioPlayer(render_config);

        // The scene is built only after the backend exists, because
        // asset-backed elements request their assets while being
        // constructed and the loaders belong to the backend.
        root_ = demo_->build_ui();
        if (root_ == nullptr) {
            throw std::runtime_error("App: demo '" + demo_->id() + "' produced an empty UI tree");
        }
    }

    App::~App() = default;

    void App::handle_event(const event::EventConcept &ev) {
        // Window events are broadcast: the renderer resizes its view,
        // App stops the loop on close.
        if (has(ev.mask(), event::EventMask::Window)) {
            bus_->emit(ev);
            return;
        }

        if (ev.type() != std::type_index(typeid(event::input::KeyPressed))) {
            return;
        }

        const auto& pressed = *static_cast<const event::input::KeyPressed*>(ev.data());

        if (pressed.key == event::input::Key::Escape) {
            running_ = false;
            return;
        }

        demo::Context ctx{*root_, *audio_, total_seconds_};
        demo_->on_key(ctx, pressed.key);
    }

    void App::run() {
        if (collector_ == nullptr || renderer_ == nullptr || audio_ == nullptr) {
            const std::string err = "App::run - the backend was not fully created";
            LOG(err);
            throw std::runtime_error(err);
        }

        {
            demo::Context ctx{*root_, *audio_, 0.f};
            demo_->on_start(ctx);
        }

        // Take the first tick before the loop so the first frame does
        // not get charged for all of the start-up time.
        intrnl::Delta_Timer::instance().tick();

        while (running_) {
            const float delta = intrnl::Delta_Timer::instance().tick();
            total_seconds_ += delta;

            demo::Context ctx{*root_, *audio_, total_seconds_};

            // 1. Time-based state: animation cursors, button
            //    transitions, demo logic.
            root_->update(delta);
            demo_->update(ctx, delta);

            // 2. Layout against the current window size, so a resize
            //    re-flows the UI instead of stretching it.
            const math::Vector2 screen_size = renderer_->screen_size();
            root_->measure(screen_size);
            root_->layout({0.f, 0.f, screen_size.x, screen_size.y});

            // 3. Collect and submit this frame's draw list.
            rnd::RenderFrame frame;
            const rnd::FrameContext frame_ctx(true);
            root_->append_render_items(frame, frame_ctx);
            renderer_->render(frame);

            // 4. Input for the next frame.
            collector_->collect();
            while (!collector_->event_store_.empty()) {
                const std::unique_ptr<event::EventConcept> ev = collector_->event_store_.pop_concept();
                handle_event(*ev);
            }
        }

        audio_->stop_all();

        // Asset loads run on detached threads; letting the program tear
        // the registry down underneath one of them is a race nobody
        // would ever reproduce on purpose.
        asset::AssetRegistry::instance().wait_for_idle();
    }
}
