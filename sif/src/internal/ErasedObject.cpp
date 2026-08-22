/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-15
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#include "sif/internal/ErasedObject.h"

namespace sif::intrnl {

    ErasedObject::ErasedObject() : storage_(nullptr), destroy_(nullptr), type_(TypeId{}) {}

    ErasedObject::ErasedObject(ErasedObject&& other) noexcept
        : storage_(other.storage_), destroy_(other.destroy_), type_(other.type_) {
        other.storage_ = nullptr;
        other.destroy_ = nullptr;
        other.type_ = 0;
    }

    ErasedObject& ErasedObject::operator=(ErasedObject&& other) noexcept {
        if (this != &other) {
            reset();
            storage_ = other.storage_;
            destroy_ = other.destroy_;
            type_ = other.type_;
            other.storage_ = nullptr;
            other.destroy_ = nullptr;
            other.type_ = 0;
        }
        return *this;
    }

    ErasedObject::~ErasedObject() {
        reset();
    }

    TypeId ErasedObject::type() const {
        return type_;
    }

    bool ErasedObject::valid() const {
        return storage_ != nullptr;
    }

    void ErasedObject::reset() {
        if (storage_) {
            destroy_(storage_);
            storage_ = nullptr;
        }
    }
} // namespace sif::intrnl