/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_TOKEN_H
#define RENDER_ENGINE_TOKEN_H
#include <cstddef>
#include <string>
#include <unordered_map>

namespace sif::ui {
enum class TokenType { OpenTag, CloseTag, SelfCloseTag, Text };

struct Token {
    TokenType type;

    std::string name;

    std::unordered_map<std::string, std::string> attributes;

    /// 1-based position of the token's '<' in its file (0 = unknown,
    /// e.g. a token built in code rather than read from a file).
    std::size_t line = 0;
    std::size_t column = 0;
};
} // namespace sif::ui

#endif // RENDER_ENGINE_TOKEN_H