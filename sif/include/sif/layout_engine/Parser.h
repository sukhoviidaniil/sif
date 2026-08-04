/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-12
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_PARSER_H
#define RENDER_ENGINE_PARSER_H

#include "sif/layout_engine/Node.h"
#include "sif/layout_engine/Token.h"

namespace sif::ui {
    class Parser {
    public:
        static std::unique_ptr<Node> parse(const std::vector<Token>& tokens);
    };
}

#endif //RENDER_ENGINE_PARSER_H