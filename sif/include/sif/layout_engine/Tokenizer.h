/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-12
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_TOKENIZER_H
#define RENDER_ENGINE_TOKENIZER_H
#include <vector>

#include "sif/layout_engine/Token.h"

namespace sif::ui {
    class Tokenizer {
    public:
        static bool is_ui_xml_file(const std::string& path);
        static std::vector<Token> tokenize(const std::string& path);
        static void save_tokens(const std::string& path, const std::vector<Token>& tokens);

    private:
        static void ensure_ui_xml_file(const std::string &path, bool must_exist = true);
    };
}

#endif //RENDER_ENGINE_TOKENIZER_H