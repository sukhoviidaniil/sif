/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-01-09
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include <exception>
#include <iostream>
#include <string>

#include "App.h"
#include "Graphics_Factory.h"

#include "demos/Demo.h"
#include "demos/DemoSupport.h"
#include "demos/HeadlessCheck.h"

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/AssetImporter.h"
#include "sif/event/Event_Bus.h"

namespace {
    using namespace sif;

    void print_usage(const char* argv0) {
        std::cout << "Usage: " << argv0 << " [demo-id] [data-dir]\n\n"
                  << "Demos:\n";
        for (const std::string& id : app::demo::demo_ids()) {
            std::cout << "  " << id << std::string(12 - id.size(), ' ')
                      << app::demo::demo_title(id) << '\n';
        }
        std::cout << "  headless    Load and verify every asset without a window (used by CI)\n"
                  << "\ndata-dir defaults to ../data/ (i.e. running from the build directory).\n";
    }

    /**
     * @brief Brings the asset system up: registry file, loaders, root dir.
     *
     * Order matters and is easy to get wrong, so it lives in one place:
     * the importer must know every asset before any UI element can
     * request one, and the backend loaders must be installed before the
     * first request actually starts a load.
     */
    void bootstrap_assets(const std::string& data_dir, const ast::RB_Config& config) {
        asset::AssetImporter& importer = asset::AssetImporter::instance();
        asset::AssetRegistry& registry = asset::AssetRegistry::instance();

        importer.load_from_file(data_dir + "bin/registry.rgst.json");
        importer.load_in_registry();

        // Asset sources are stored relative to the data root, so images
        // and audio can share one registry.
        registry.set_asset_dir(data_dir);
        registry.set_max_concurrent_loads(4);

        sif::backend::Graphics_Factory::instance().register_asset_loaders(config, registry);
    }
}

int main(const int argc, char* argv[]) {
    const std::string demo_id = argc > 1 ? argv[1] : "all";
    const std::string data_dir = argc > 2 ? argv[2] : "data/";

    if (demo_id == "-h" || demo_id == "--help" || demo_id == "help") {
        print_usage(argv[0]);
        return 0;
    }

    try {
        app::demo::Paths::set_data_dir(data_dir);

        // The headless check must not touch the GPU or the sound card,
        // so it runs on the CPU-only backend instead of SFML.
        const bool headless = demo_id == "headless";

        const sif::ast::RB_Config render_config{
            .type = headless ? sif::ast::RB_Type::Headless : sif::ast::RB_Type::SFML,
            .window_name = "sif - " + demo_id,
            .window_width = 1000,
            .window_height = 640,
            .fps = 60
        };
        const sif::ast::EC_Config collector_config{ .type = sif::ast::RB_Type::SFML };

        bootstrap_assets(app::demo::Paths::data_dir(), render_config);

        if (headless) {
            return app::demo::run_headless_check();
        }

        std::unique_ptr<app::demo::Demo> demo = app::demo::make_demo(demo_id);
        if (demo == nullptr) {
            std::cerr << "Unknown demo: '" << demo_id << "'\n\n";
            print_usage(argv[0]);
            return 2;
        }

        std::cout << "Running demo '" << demo->id() << "': " << demo->title() << '\n';

        const auto bus = std::make_shared<sif::event::Event_Bus>();
        app::App application(bus, render_config, collector_config, std::move(demo));
        application.run();

        return 0;
    } catch (const std::exception& e) {
        // Missing files, malformed scenes and unsupported backends all
        // arrive here as exceptions; reporting them beats the previous
        // behaviour of letting them reach std::terminate.
        std::cerr << "Fatal: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Fatal: unknown error\n";
        return 1;
    }
}
