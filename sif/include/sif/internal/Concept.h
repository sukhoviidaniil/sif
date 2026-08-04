/***************************************************************
 * Project:       Render_Engine
 * File:          Concept.h
 *
 * Author:        Sukhovii Daniil
 * Created:       2026-01-15
 * Modified:      []
 *
 * Description:   []
 *
 * Contact:
 *   Email:       sukhovii.daniil@gmail.com
 *
 * Disclaimer:
 *   This file is part of Render_Engine.
 *   Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_CONCEPT_H
#define RENDER_ENGINE_CONCEPT_H
#include <algorithm>

namespace sif::intrnl {
    struct Concept {
        virtual ~Concept() = default;
    };

    template<typename T>
    struct Model final : Concept {
        explicit Model(T v) : impl(std::move(v)) {}
        T impl;
    };
}

#endif //RENDER_ENGINE_CONCEPT_H