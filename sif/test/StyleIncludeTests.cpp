/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-11-05
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "TestFramework.h"
#include "Fixtures.h"

#include <filesystem>
#include <string>
#include <vector>

#include "sif/layout_engine/Parser.h"
#include "sif/layout_engine/SceneComposer.h"
#include "sif/layout_engine/Tokenizer.h"
#include "sif/layout_engine/UIFactory.h"
#include "sif/layout_engine/elements/Overlay.h"
#include "sif/layout_engine/elements/Rectangle.h"

using namespace sif;
namespace fs = std::filesystem;

namespace {
    /// @brief A fresh scratch directory (under the system temp dir, not the CWD).
    fs::path scratch(const std::string& name) {
        const fs::path dir = fs::temp_directory_path() / ("sif_tests_" + name);
        fs::remove_all(dir);
        fs::create_directories(dir);
        return dir;
    }

    void write(const fs::path& file, const std::string& content) {
        fs::create_directories(file.parent_path());
        sif::test::fixtures::write_file(file.string(), content);
    }

    std::string load_error(const fs::path& file) {
        try {
            (void)ui::SceneLoader::load(file.string());
        } catch (const std::exception& e) {
            return e.what();
        }
        return {};
    }

    bool contains(const std::string& haystack, const std::string& needle) {
        return haystack.find(needle) != std::string::npos;
    }

    struct StrictScope {
        StrictScope() { ui::UIFactory::instance().set_strict(true); }
        ~StrictScope() { ui::UIFactory::instance().set_strict(false); }
    };
}

SIF_TEST(styles_apply_in_class_order_and_own_attributes_win) {
    const fs::path dir = scratch("styles");
    write(dir / "scene.ui.xml", R"(<Overlay name="root">
    <Style name="red"  fill_color="200|0|0" width="PX|10"/>
    <Style name="wide" width="PX|300" height="PX|20"/>
    <Rectangle name="a" class="red wide"/>
    <Rectangle name="b" class="wide red"/>
    <Rectangle name="c" class="red wide" width="PX|5"/>
</Overlay>)");

    const auto root = ui::SceneLoader::load((dir / "scene.ui.xml").string());
    auto* a = root->find_by_name<ui::Rectangle>("a");
    auto* b = root->find_by_name<ui::Rectangle>("b");
    auto* c = root->find_by_name<ui::Rectangle>("c");
    if (!a || !b || !c) { SIF_CHECK(false); return; }

    SIF_CHECK(a->fill_color.r == 200 && a->width.value == 300.f && a->height.value == 20.f); // later class wins
    SIF_CHECK(b->width.value == 10.f);                                                       // ...in either order
    SIF_CHECK(c->width.value == 5.f);                                                        // own attribute wins
    SIF_CHECK(root->children.size() == 3);                                                   // Style nodes are gone
}

SIF_TEST(includes_are_relative_nested_and_overridable) {
    const fs::path dir = scratch("includes");
    write(dir / "parts" / "leaf.ui.xml", R"(<Rectangle name="leaf" class="tinted"/>)");
    write(dir / "parts" / "panel.ui.xml", R"(<Overlay name="panel" width="PX|100">
    <Style name="tinted" fill_color="0|0|200"/>
    <Include src="leaf.ui.xml"/>
</Overlay>)");
    write(dir / "scene.ui.xml", R"(<Overlay name="root">
    <Include src="parts/panel.ui.xml" name="left"/>
    <Include src="parts/panel.ui.xml" name="right" width="PX|50"/>
</Overlay>)");

    std::vector<std::string> warnings;
    const auto root = ui::SceneLoader::load((dir / "scene.ui.xml").string(), &warnings);
    auto* left = root->find_by_name<ui::Overlay>("left");
    auto* right = root->find_by_name<ui::Overlay>("right");
    if (!left || !right) { SIF_CHECK(false); return; }

    SIF_CHECK(left->width.value == 100.f);
    SIF_CHECK(right->width.value == 50.f); // overridden by the <Include>
    SIF_CHECK(root->find_by_name<ui::Overlay>("panel") == nullptr);
    // A style declared inside an included file applies document-wide.
    auto* leaf = left->find_by_name<ui::Rectangle>("leaf");
    SIF_CHECK(leaf != nullptr && leaf->fill_color.b == 200);
    SIF_CHECK(warnings.empty());
}

SIF_TEST(include_errors_are_reported_with_their_file) {
    const fs::path dir = scratch("include_errors");

    write(dir / "a.ui.xml", R"(<Overlay><Include src="b.ui.xml"/></Overlay>)");
    write(dir / "b.ui.xml", R"(<Overlay><Include src="a.ui.xml"/></Overlay>)");
    SIF_CHECK(contains(load_error(dir / "a.ui.xml"), "include cycle"));

    write(dir / "missing.ui.xml", "<Overlay>\n  <Include src=\"nowhere.ui.xml\"/>\n</Overlay>");
    const std::string missing = load_error(dir / "missing.ui.xml");
    SIF_CHECK(contains(missing, "missing.ui.xml, line 2, column 3"));
    SIF_CHECK(contains(missing, "cannot include"));

    write(dir / "no_src.ui.xml", R"(<Overlay><Include/></Overlay>)");
    SIF_CHECK(contains(load_error(dir / "no_src.ui.xml"), "needs a src attribute"));

    // An error inside an included file names that file, not the includer.
    write(dir / "bad_part.ui.xml", "<Overlay>\n  <Rectangle fill_color=\"red\"/>\n</Overlay>");
    write(dir / "uses_bad.ui.xml", R"(<Overlay><Include src="bad_part.ui.xml"/></Overlay>)");
    const std::string inner = load_error(dir / "uses_bad.ui.xml");
    SIF_CHECK(contains(inner, "bad_part.ui.xml, line 2, column 3"));
}

SIF_TEST(style_errors_and_unknown_classes) {
    const fs::path dir = scratch("style_errors");

    write(dir / "dup.ui.xml", R"(<Overlay><Style name="x"/><Style name="x"/></Overlay>)");
    SIF_CHECK(contains(load_error(dir / "dup.ui.xml"), "style 'x' is already defined"));

    write(dir / "nameless.ui.xml", R"(<Overlay><Style fill_color="1|1|1"/></Overlay>)");
    SIF_CHECK(contains(load_error(dir / "nameless.ui.xml"), "needs a name attribute"));

    write(dir / "unknown.ui.xml", R"(<Overlay><Rectangle class="nope"/></Overlay>)");
    std::vector<std::string> warnings;
    (void)ui::SceneLoader::load((dir / "unknown.ui.xml").string(), &warnings);
    SIF_CHECK(warnings.size() == 1 && contains(warnings[0], "unknown class 'nope'"));

    const StrictScope strict;
    SIF_CHECK(contains(load_error(dir / "unknown.ui.xml"), "unknown class 'nope'"));
}

SIF_TEST(building_an_uncomposed_tree_explains_what_is_missing) {
    const fs::path dir = scratch("uncomposed");
    write(dir / "scene.ui.xml", R"(<Overlay><Style name="x"/></Overlay>)");
    const auto node = ui::Parser::parse(ui::Tokenizer::tokenize((dir / "scene.ui.xml").string()));
    std::string error;
    try {
        (void)ui::UIFactory::instance().build(*node);
    } catch (const ui::SceneError& e) {
        error = e.what();
    }
    SIF_CHECK(contains(error, "must be resolved before building"));
}
