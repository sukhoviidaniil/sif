/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_VIEW_AST_NODE_H
#define RENDER_ENGINE_VIEW_AST_NODE_H

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sif::ui {
struct Node {
    std::string tag;

    std::unordered_map<std::string, std::string> attributes;

    std::vector<std::unique_ptr<Node>> children;

    /// 1-based position of the tag in its file, carried over from the
    /// Token (0 = unknown), so errors found while *building* the
    /// element can still point at the line that caused them.
    std::size_t line = 0;
    std::size_t column = 0;

    /// File the node was read from (empty = unknown); set by
    /// SceneComposer, so errors inside an included file name that file.
    std::string source;
};
} // namespace sif::ui

#endif // RENDER_ENGINE_VIEW_AST_NODE_H