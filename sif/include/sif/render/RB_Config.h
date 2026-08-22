/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-08-03
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_RB_CONFIG_H
#define RENDER_ENGINE_RB_CONFIG_H

#include <cstdint>
#include <string>

namespace sif::ast {
    /**
     * @brief Which rendering backend a Renderer should be built for.
     *
     * Kept in the engine (not in the SFML layer) on purpose: the
     * abstract factory in the representation layer switches on this
     * value, but the engine itself never needs to know what "SFML"
     * means beyond "some backend id".
     */
    enum class RB_Type {
        None,
        SFML,
        /**
         * @brief CPU-only backend: assets are decoded, nothing is
         * drawn and no output device is opened.
         *
         * Exists so the asset pipeline can be exercised on machines
         * with no display and no sound card (CI), and as the practical
         * proof that swapping backends touches no engine code.
         */
        Headless
    };

    inline RB_Type rb_type_from_string(const std::string& s) {
        if (s == "SFML")
            return RB_Type::SFML;
        if (s == "Headless")
            return RB_Type::Headless;
        return RB_Type::None;
    }

    inline std::string to_string(const RB_Type t) {
        switch (t) {
        case RB_Type::SFML:
            return "SFML";
        case RB_Type::Headless:
            return "Headless";
        default:
            return "None";
        }
    }

    /**
     * @brief Backend-agnostic description of the render target/window.
     *
     * A plain DTO handed to the concrete factory; every field is
     * explicitly initialized so a partially-filled designated
     * initializer can never leave garbage behind.
     */
    struct RB_Config {
        RB_Type type = RB_Type::None;
        std::string window_name = "sif";
        unsigned int window_width = 800;
        unsigned int window_height = 600;
        int fps = 60;
    };

    /**
     * @brief Backend-agnostic description of the event source.
     *
     * Currently the event collector is always bound to the window
     * created by the renderer, so this only carries the backend id;
     * it exists so App does not have to guess which collector to
     * build (see Graphics_Factory::make_Event_Collector).
     */
    struct EC_Config {
        RB_Type type = RB_Type::None;
    };
} // namespace sif::ast

#endif // RENDER_ENGINE_RB_CONFIG_H
