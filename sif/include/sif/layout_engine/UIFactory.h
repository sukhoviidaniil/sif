/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-12
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_UIFACTORY_H
#define RENDER_ENGINE_UIFACTORY_H

#include "sif/layout_engine/Node.h"
#include "sif/layout_engine/elements/UIElement.h"

namespace sif::ui {

    class UIFactory {
    public:
        static UIFactory& instance();

        void register_tag(std::string tag, std::function<std::unique_ptr<UIElement>(const Node&)> fn);

        std::unique_ptr<UIElement> build(const Node& n);

    private:
        std::unordered_map<std::string, std::function<std::unique_ptr<UIElement>(const Node&)>> map_;

        UIFactory();
    };
} // namespace sif::ui

#endif // RENDER_ENGINE_UIFACTORY_H