/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-02-25
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "Graphics_Factory.h"

#include <stdexcept>

#include "headless/HeadlessBackend.h"
#include "sfml/SFMLAssetLoaders.h"
#include "sfml/SFML_AudioPlayer.h"
#include "sfml/SFML_Event_Collector.h"
#include "sfml/SFML_Renderer.h"

namespace app {
    using namespace sif;

    Graphics_Factory & Graphics_Factory::instance() {
        static Graphics_Factory inst;
        return inst;
    }

    Graphics_Factory::Graphics_Factory() = default;

    std::shared_ptr<rnd::Renderer> Graphics_Factory::make_Renderer(const ast::RB_Config &config) {
        switch (config.type) {
            case ast::RB_Type::SFML: {
                auto renderer = std::make_shared<sfml::SFML_Renderer>(config);
                last_renderer_ = renderer;
                return renderer;
            }
            default:
                throw std::runtime_error(
                    "Graphics_Factory::make_Renderer - unsupported backend: " + ast::to_string(config.type)
                );
        }
    }

    std::unique_ptr<event::Event_Collector> Graphics_Factory::make_Event_Collector(const ast::EC_Config &config) {
        switch (config.type) {
            case ast::RB_Type::SFML: {
                const std::shared_ptr<rnd::Renderer> renderer = last_renderer_.lock();
                if (renderer == nullptr) {
                    throw std::runtime_error(
                        "Graphics_Factory::make_Event_Collector - create the renderer first: "
                        "the SFML event stream belongs to its window"
                    );
                }
                // Safe downcast: this branch only ever sees a renderer
                // this same factory built for RB_Type::SFML.
                auto* source = static_cast<sfml::SFML_Renderer*>(renderer.get());
                return std::make_unique<sfml::SFML_Event_Collector>(*source);
            }
            default:
                throw std::runtime_error(
                    "Graphics_Factory::make_Event_Collector - unsupported backend: " + ast::to_string(config.type)
                );
        }
    }

    std::shared_ptr<audio::AudioPlayer> Graphics_Factory::make_AudioPlayer(const ast::RB_Config &config) {
        switch (config.type) {
            case ast::RB_Type::SFML:
                return std::make_shared<sfml::SFML_AudioPlayer>();
            default:
                throw std::runtime_error(
                    "Graphics_Factory::make_AudioPlayer - unsupported backend: " + ast::to_string(config.type)
                );
        }
    }

    void Graphics_Factory::register_asset_loaders(const ast::RB_Config &config, asset::AssetRegistry &registry) {
        switch (config.type) {
            case ast::RB_Type::SFML:
                sfml::register_sfml_asset_loaders(registry);
                break;
            case ast::RB_Type::Headless:
                headless::register_headless_asset_loaders(registry);
                break;
            default:
                throw std::runtime_error(
                    "Graphics_Factory::register_asset_loaders - unsupported backend: " + ast::to_string(config.type)
                );
        }
    }
}
