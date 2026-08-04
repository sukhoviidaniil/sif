/***************************************************************
* Project:       Render_Engine
 * File:          ErasedObject.inl
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

#pragma once

#include <algorithm>
#include <iostream>


namespace sif::intrnl {

    template<typename T>
    ErasedObject::ErasedObject(T value)
    : storage_(new T(std::move(value)))
    , destroy_(&destroy_impl<T>)
    , type_(type_id<T>()) {
    }

    inline void assert_true(bool condition, const char* message) {
        if (!condition) {
            std::cerr << "Assertion failed: " << message << std::endl;
            std::abort();
        }
    }

    template<typename T>
    T& ErasedObject::get() {
        assert_true(type_ == type_id<T>(), "Bad ErasedObject type access");
        return *static_cast<T*>(storage_);
    }

    template<typename T>
    const T& ErasedObject::get() const {
        assert_true(type_ == type_id<T>(), "Bad ErasedObject type access");
        return *static_cast<const T*>(storage_);
    }

    template<typename T>
    void ErasedObject::destroy_impl(void* ptr) {
        delete static_cast<T*>(ptr);
    }
}
