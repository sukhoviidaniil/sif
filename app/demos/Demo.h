/***************************************************************
 * Project:       Render_Engine (demos)
 * File:          Demo.h
 *
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_DEMO_H
#define RENDER_ENGINE_DEMO_H

#include <memory>
#include <string>
#include <vector>

#include "sif/audio/AudioPlayer.h"
#include "sif/event/events/input_Keyboard.h"
#include "sif/layout_engine/elements/UIElement.h"

namespace app::demo {
    /**
     * @brief Everything a demo is allowed to touch while it runs.
     *
     * Deliberately small: a demo may reshape its own UI subtree and
     * make noise, and that is all. It gets no window, no renderer and
     * no event loop, which is what keeps the demos honest - if a demo
     * can show a sprite, it is because the engine can, not because the
     * demo reached around it.
     */
    struct Context {
        sif::ui::UIElement& root;
        sif::audio::AudioPlayer& audio;
        float total_seconds = 0.f;
    };

    /**
     * @brief One self-contained scenario shown by the demo application.
     *
     * Template Method: App owns the frame loop and calls these hooks at
     * fixed points (build_ui, then on_start, then update/on_key every
     * frame), so every demo shares the exact same asset-loading,
     * layout, render and input path. A bug that only reproduces in one
     * demo is therefore a bug in that demo's content, not in how it was
     * driven.
     */
    class Demo {
    public:
        virtual ~Demo();

        /// @brief Command-line id, e.g. "sprites".
        [[nodiscard]] virtual std::string id() const = 0;

        /// @brief One-line description printed by --help and on screen.
        [[nodiscard]] virtual std::string title() const = 0;

        /**
         * @brief Builds this demo's UI tree.
         *
         * Called once, after the asset registry is populated and the
         * backend loaders are installed, so asset-backed elements can
         * be created here directly.
         */
        [[nodiscard]] virtual std::unique_ptr<sif::ui::UIElement> build_ui() = 0;

        /// @brief Called once, right after the tree is built and laid out.
        virtual void on_start(Context& ctx);

        /// @brief Called once per frame with the measured frame time.
        virtual void update(Context& ctx, float dt);

        /// @brief Called for every key press that is not a global shortcut.
        virtual void on_key(Context& ctx, sif::event::input::Key key);
    };

    /// @brief Ids of every registered demo, in menu order.
    [[nodiscard]] std::vector<std::string> demo_ids();

    /// @brief One-line description of a demo id (empty if unknown).
    [[nodiscard]] std::string demo_title(const std::string& id);

    /**
     * @brief Creates a demo by id.
     *
     * @return nullptr if the id is unknown, so main() can print the
     * available ids instead of throwing at the user.
     */
    [[nodiscard]] std::unique_ptr<Demo> make_demo(const std::string& id);
} // namespace app::demo

#endif // RENDER_ENGINE_DEMO_H
