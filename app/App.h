/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-02-25
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_APP_H
#define RENDER_ENGINE_APP_H

#include <memory>

#include "demos/Demo.h"

#include "sif/audio/AudioPlayer.h"
#include "sif/event/Event_Bus.h"
#include "sif/event/Event_Collector.h"
#include "sif/event/Observer.h"
#include "sif/layout_engine/elements/UIElement.h"
#include "sif/render/RB_Config.h"
#include "sif/render/Renderer.h"

namespace app {
    /**
     * @brief Owns the window, the frame loop and the event plumbing.
     *
     * Everything App does is backend-independent: it asks the abstract
     * factory for a renderer/collector/audio player, walks the UI tree
     * (update -> measure -> layout -> collect render items -> render)
     * and forwards events. It contains no drawing code and no game
     * rules, which is the separation the assignment asks for.
     *
     * App is an Observer of the global bus so that window events
     * (close, resize) reach it and the renderer through the same
     * mechanism as any other event, rather than through a special case.
     */
    class App final : public sif::event::Observer {
    public:
        App(std::shared_ptr<sif::event::Event_Bus> bus, const sif::ast::RB_Config& render_config,
            const sif::ast::EC_Config& collector_config, std::unique_ptr<demo::Demo> demo);

        ~App() override;

        /**
         * @brief Runs until the window is closed or Escape is pressed.
         *
         * The loop is frame-rate independent: every time-based value
         * (animation cursors, button transitions, demo logic) is scaled
         * by the measured delta from the Stopwatch singleton, and the
         * only throttling used is the backend's frame-rate cap.
         */
        void run();

    private:
        /// @brief Routes one collected event to the bus or to the demo.
        void handle_event(const sif::event::EventConcept& ev);

        bool running_ = true;

        std::shared_ptr<sif::event::Event_Bus> bus_;
        std::shared_ptr<sif::rnd::Renderer> renderer_;
        std::unique_ptr<sif::event::Event_Collector> collector_;
        std::shared_ptr<sif::audio::AudioPlayer> audio_;

        std::unique_ptr<demo::Demo> demo_;
        std::unique_ptr<sif::ui::UIElement> root_;

        float total_seconds_ = 0.f;
    };
} // namespace app

#endif // RENDER_ENGINE_APP_H
