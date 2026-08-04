/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-12
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/layout_engine/Parser.h"

#include <stack>
#include <stdexcept>

namespace sif::ui {
    std::unique_ptr<Node> Parser::parse(const std::vector<Token>& tokens) {
        std::stack<Node*> stack;
        std::unique_ptr<Node> root;

        for (auto& t : tokens) {
            switch (t.type) {
                case TokenType::OpenTag:
                case TokenType::SelfCloseTag: {
                    auto node = std::make_unique<Node>();
                    node->tag = t.name;
                    node->attributes = t.attributes;

                    Node* raw = node.get();

                    if (!stack.empty()) {
                        stack.top()->children.push_back(std::move(node));
                    }else {
                        root = std::move(node);
                    }


                    if (t.type == TokenType::OpenTag) {
                        stack.push(raw);
                    }

                    break;
                }
                case TokenType::CloseTag: {
                    // A stray '</X>' used to pop an empty stack, which
                    // is undefined behaviour; a malformed scene file is
                    // now a diagnosable error instead of a crash.
                    if (stack.empty()) {
                        throw std::runtime_error(
                            "UI parse error: closing tag '</" + t.name + ">' without a matching opening tag"
                        );
                    }
                    if (stack.top()->tag != t.name) {
                        throw std::runtime_error(
                            "UI parse error: '</" + t.name + ">' closes '<" + stack.top()->tag + ">'"
                        );
                    }
                    stack.pop();
                    break;
                }
                default:
                    break;
            }
        }

        return root;
    }
}
