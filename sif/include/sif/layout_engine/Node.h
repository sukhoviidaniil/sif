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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sif::ui {
    struct Node {
        std::string tag;

        std::unordered_map<std::string, std::string> attributes;

        std::vector<std::unique_ptr<Node>> children;
    };
} // namespace sif::ui

#endif // RENDER_ENGINE_VIEW_AST_NODE_H