/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-11-05
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/

#include "sif/layout_engine/SceneComposer.h"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <sstream>
#include <unordered_map>

#include "sif/diagnostics/Logger.h"
#include "sif/layout_engine/Parser.h"
#include "sif/layout_engine/Tokenizer.h"
#include "sif/layout_engine/UIFactory.h"

namespace sif::ui {
namespace {
namespace fs = std::filesystem;

std::string where(const Node& n) {
    std::string out = n.source;
    if (n.line != 0) {
        out += (out.empty() ? "" : ", ") + std::string("line ") + std::to_string(n.line) + ", column " +
               std::to_string(n.column);
    }
    return out.empty() ? std::string{} : out + ": ";
}

void stamp_source(Node& n, const std::string& source) {
    n.source = source;
    for (auto& c : n.children) {
        stamp_source(*c, source);
    }
}

std::unique_ptr<Node> parse_file(const std::string& path) {
    auto root = Parser::parse(Tokenizer::tokenize(path));
    if (root == nullptr) {
        throw SceneError(path + ": contains no root element");
    }
    stamp_source(*root, path);
    return root;
}

/**
 * @brief Replaces every <Include> under n (recursively) by its file's root.
 *
 * @param dir Directory of the file n came from.
 * @param stack Canonical paths of the files currently being included,
 * outermost first - a file appearing twice is a cycle.
 */
void expand_includes(Node& n, const fs::path& dir, std::vector<std::string>& stack) {
    for (auto& child : n.children) {
        if (child->tag != "Include") {
            expand_includes(*child, dir, stack);
            continue;
        }

        if (!child->attributes.contains("src")) {
            throw SceneError(where(*child) + "<Include> needs a src attribute");
        }
        if (!child->children.empty()) {
            throw SceneError(where(*child) + "<Include> cannot have children");
        }

        const fs::path file = (dir / child->attributes.at("src")).lexically_normal();
        std::error_code ec;
        const fs::path canonical = fs::weakly_canonical(file, ec);
        const std::string key = (ec ? file : canonical).string();

        if (std::find(stack.begin(), stack.end(), key) != stack.end()) {
            std::string chain;
            for (const std::string& s : stack) {
                chain += s + " -> ";
            }
            throw SceneError(where(*child) + "include cycle: " + chain + key);
        }

        std::unique_ptr<Node> included;
        try {
            included = parse_file(file.string());
        } catch (const SceneError&) {
            throw;
        } catch (const std::exception& ex) {
            throw SceneError(where(*child) + "cannot include '" + file.string() + "': " + ex.what());
        }

        stack.push_back(key);
        expand_includes(*included, file.parent_path(), stack);
        stack.pop_back();

        // The <Include>'s own attributes win over the included root's.
        for (const auto& [attribute, value] : child->attributes) {
            if (attribute != "src") {
                included->attributes[attribute] = value;
            }
        }
        child = std::move(included);
    }
}

/// @brief Moves every <Style> out of the tree into styles (by name).
void collect_styles(Node& n, std::unordered_map<std::string, std::unique_ptr<Node>>& styles) {
    for (auto it = n.children.begin(); it != n.children.end();) {
        Node& child = **it;
        if (child.tag != "Style") {
            collect_styles(child, styles);
            ++it;
            continue;
        }
        if (!child.attributes.contains("name") || child.attributes.at("name").empty()) {
            throw SceneError(where(child) + "<Style> needs a name attribute");
        }
        if (!child.children.empty()) {
            throw SceneError(where(child) + "<Style> cannot have children");
        }
        const std::string name = child.attributes.at("name");
        if (const auto existing = styles.find(name); existing != styles.end()) {
            const Node& first = *existing->second;
            // A file included twice brings its styles twice: that is the
            // same definition, not a clash.
            const bool same_definition =
                first.source == child.source && first.line == child.line && first.column == child.column;
            if (!same_definition) {
                const std::string at = where(first);
                throw SceneError(where(child) + "style '" + name + "' is already defined at " +
                                 at.substr(0, at.size() - 2));
            }
            it = n.children.erase(it);
            continue;
        }
        styles.emplace(name, std::move(*it));
        it = n.children.erase(it);
    }
}

void apply_classes(Node& n, const std::unordered_map<std::string, std::unique_ptr<Node>>& styles,
                   std::vector<std::string>* warnings, const bool strict) {
    if (const auto it = n.attributes.find("class"); it != n.attributes.end()) {
        std::unordered_map<std::string, std::string> merged;

        std::istringstream classes(it->second);
        std::string name;
        while (classes >> name) {
            const auto style = styles.find(name);
            if (style == styles.end()) {
                const std::string message = where(n) + "<" + n.tag + "> uses unknown class '" + name + "'";
                if (strict) {
                    throw SceneError(message);
                }
                LOG("Warning: " + message);
                if (warnings != nullptr) {
                    warnings->push_back(message);
                }
                continue;
            }
            for (const auto& [attribute, value] : style->second->attributes) {
                if (attribute != "name") {
                    merged[attribute] = value; // later classes win
                }
            }
        }

        n.attributes.erase(it);
        for (auto& [attribute, value] : n.attributes) {
            merged[attribute] = value; // own attributes win over every class
        }
        n.attributes = std::move(merged);
    }

    for (auto& c : n.children) {
        apply_classes(*c, styles, warnings, strict);
    }
}
} // namespace

void SceneComposer::compose(Node& root, const std::string& source_path, std::vector<std::string>* warnings) {
    if (root.tag == "Include" || root.tag == "Style") {
        throw SceneError(where(root) + "<" + root.tag + "> cannot be the root of a scene");
    }
    if (root.source.empty()) {
        stamp_source(root, source_path);
    }

    std::error_code ec;
    const fs::path path(source_path);
    const fs::path canonical = fs::weakly_canonical(path, ec);
    std::vector<std::string> stack{(ec ? path : canonical).string()};
    expand_includes(root, path.parent_path(), stack);

    std::unordered_map<std::string, std::unique_ptr<Node>> styles;
    collect_styles(root, styles);

    apply_classes(root, styles, warnings, UIFactory::instance().strict());
}

std::unique_ptr<UIElement> SceneLoader::load(const std::string& path, std::vector<std::string>* warnings) {
    const std::unique_ptr<Node> root = parse_file(path);
    SceneComposer::compose(*root, path, warnings);
    std::unique_ptr<UIElement> element = UIFactory::instance().build(*root);
    if (warnings != nullptr) {
        const std::vector<std::string>& built = UIFactory::instance().warnings();
        warnings->insert(warnings->end(), built.begin(), built.end());
    }
    return element;
}
} // namespace sif::ui
