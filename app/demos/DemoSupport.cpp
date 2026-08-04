/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "DemoSupport.h"

#include "Demo.h"

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/AssetImporter.h"
#include "sif/layout_engine/Parser.h"
#include "sif/layout_engine/Tokenizer.h"
#include "sif/layout_engine/UIFactory.h"
#include "sif/layout_engine/elements/Text.h"

namespace app::demo {
    using namespace sif;

    namespace {
        std::string& data_dir_storage() {
            static std::string dir = "data/";
            return dir;
        }
    }

    void Paths::set_data_dir(const std::string &dir) {
        std::string normalized = dir;
        if (!normalized.empty() && normalized.back() != '/') {
            normalized.push_back('/');
        }
        data_dir_storage() = normalized;
    }

    const std::string & Paths::data_dir() {
        return data_dir_storage();
    }

    std::string Paths::scene(const std::string &file) {
        return data_dir() + "bin/scenes/" + file;
    }

    std::unique_ptr<ui::UIElement> load_scene(const std::string &scene_file) {
        const std::vector<ui::Token> tokens = ui::Tokenizer::tokenize(Paths::scene(scene_file));
        const std::unique_ptr<ui::Node> node_root = ui::Parser::parse(tokens);
        if (node_root == nullptr) {
            throw std::runtime_error("Scene '" + scene_file + "' contains no root element");
        }
        return ui::UIFactory::instance().build(*node_root);
    }

    intrnl::GUID guid_of(const std::string &asset_name) {
        return asset::AssetImporter::instance().get(asset_name).meta.guid;
    }

    asset::AssetHandle<asset::Sound> request_sound(const std::string &asset_name) {
        const intrnl::GUID guid = guid_of(asset_name);
        asset::AssetRegistry::instance().request(guid);
        return asset::AssetRegistry::instance().get<asset::Sound>(guid);
    }

    void set_label(ui::UIElement &root, const std::string &element_name, const std::string &value) {
        if (auto* label = root.find_by_name<ui::Text>(element_name)) {
            label->text = value;
        }
    }

    Demo::~Demo() = default;

    void Demo::on_start(Context& /*ctx*/) {
    }

    void Demo::update(Context& /*ctx*/, float /*dt*/) {
    }

    void Demo::on_key(Context& /*ctx*/, event::input::Key /*key*/) {
    }
}
