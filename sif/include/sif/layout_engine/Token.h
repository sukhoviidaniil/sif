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
#include <string>
#include <unordered_map>

namespace sif::ui {
    enum class TokenType {
        OpenTag,
        CloseTag,
        SelfCloseTag,
        Text
    };

    struct Token {
        TokenType type;

        std::string name;

        std::unordered_map<
            std::string,
            std::string
        > attributes;
    };
}

#endif //RENDER_ENGINE_TOKEN_H