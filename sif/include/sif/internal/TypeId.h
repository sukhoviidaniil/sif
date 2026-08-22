/***************************************************************
 * Project:       Render_Engine
 * File:          TypeId.h
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
#ifndef RENDER_ENGINE_TYPEID_H
#define RENDER_ENGINE_TYPEID_H
#include <cstddef>

namespace sif::intrnl {
    using TypeId = std::size_t;

    template<typename T>
    TypeId type_id() {
        static int unique;
        return reinterpret_cast<TypeId>(&unique);
    }
} // namespace sif::intrnl

#endif // RENDER_ENGINE_TYPEID_H