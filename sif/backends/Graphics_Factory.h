/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-02-25
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_GRAPHICS_FACTORY_H
#define RENDER_ENGINE_GRAPHICS_FACTORY_H

#include <memory>

#include "sif/asset/AssetRegistry.h"
#include "sif/audio/AudioPlayer.h"
#include "sif/event/Event_Collector.h"
#include "sif/render/RB_Config.h"
#include "sif/render/Renderer.h"

/**
 * @brief Abstract Factory over one whole rendering/audio backend.
 *
 * Lives in sif::backend (not in a bare `app` namespace) because it ships
 * with sif itself now, as part of the promoted sif_sfml target - a
 * consumer such as Bomberman links one library and gets a renderer, an
 * event collector, an audio player and every asset loader, instead of
 * hand-compiling sif's app/sfml and app/headless sources itself.
 */
namespace sif::backend {
    /**
     * @brief Abstract Factory over one whole rendering/audio backend.
     *
     * The engine (sif) declares *what* it needs - a Renderer, an
     * Event_Collector, an AudioPlayer, a loader per AssetType - and
     * never learns which library provides them. Choosing a backend is
     * a single value (ast::RB_Type) passed into this factory, so
     * adding e.g. an SDL backend means adding one branch here plus the
     * classes it returns, and touching nothing in sif/.
     *
     * Why the collector cannot be built independently: SFML delivers
     * OS events through the window, and the window is owned by the
     * renderer. make_Event_Collector therefore binds to the renderer
     * created by the most recent make_Renderer call - the coupling is
     * inherent to the platform, and confining it to this one class is
     * the point.
     *
     * Singleton because the backend choice is a process-wide fact, and
     * because the asset loaders it installs are registered into the
     * (equally process-wide) AssetRegistry.
     */
    class Graphics_Factory {
    public:
        static Graphics_Factory& instance();

        Graphics_Factory(const Graphics_Factory&) = delete;
        Graphics_Factory& operator=(const Graphics_Factory&) = delete;

        /**
         * @brief Creates the render backend described by config.
         *
         * @throws std::runtime_error for an unknown/None backend type.
         */
        std::shared_ptr<sif::rnd::Renderer> make_Renderer(const sif::ast::RB_Config& config);

        /**
         * @brief Creates the event collector bound to the last renderer.
         *
         * @throws std::runtime_error if no renderer has been created yet.
         */
        std::unique_ptr<sif::event::Event_Collector> make_Event_Collector(const sif::ast::EC_Config& config);

        /**
         * @brief Creates the audio backend for the given backend type.
         *
         * @throws std::runtime_error for an unknown/None backend type.
         */
        std::shared_ptr<sif::audio::AudioPlayer> make_AudioPlayer(const sif::ast::RB_Config& config);

        /**
         * @brief Installs one asset loader per AssetType into the registry.
         *
         * Idempotent: calling it twice simply replaces the loaders.
         */
        void register_asset_loaders(const sif::ast::RB_Config& config,
                                    sif::asset::AssetRegistry& registry);

    private:
        Graphics_Factory();

        /// Non-owning: the renderer's lifetime belongs to whoever asked for it.
        std::weak_ptr<sif::rnd::Renderer> last_renderer_;
    };
}

#endif //RENDER_ENGINE_GRAPHICS_FACTORY_H
