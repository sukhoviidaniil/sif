/***************************************************************
 * Project:       Render_Engine
 * File:          ErasedObject.h
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
#ifndef RENDER_ENGINE_ERASEDOBJECT_H
#define RENDER_ENGINE_ERASEDOBJECT_H

#include "TypeId.h"

namespace sif::intrnl {
    class ErasedObject {
    public:
        ErasedObject(const ErasedObject&) = delete;
        ErasedObject& operator=(const ErasedObject&) = delete;

        ErasedObject();

        /**
         *
         * @tparam T Movement and the move constructor should be allowed.
         * @param value
         */
        template<typename T>
        explicit ErasedObject(T value);

        ErasedObject(ErasedObject&& other) noexcept;

        ErasedObject& operator=(ErasedObject&& other) noexcept;

        ~ErasedObject();

        template<typename T>
        T& get();

        template<typename T>
        const T& get() const;

        [[nodiscard]] TypeId type() const;

        [[nodiscard]] bool valid() const;

    private:

        void reset();

        template<typename T>
        static void destroy_impl(void* ptr);

        // Implementation of unique_ptr, but for a dynamic type
        void* storage_;
        void (*destroy_)(void*);

        TypeId type_;
    };
}

#include "sif/internal/ErasedObject.inl"

#endif //RENDER_ENGINE_ERASEDOBJECT_H