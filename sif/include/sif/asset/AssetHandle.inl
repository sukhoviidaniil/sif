/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-13
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#pragma once
#include <utility>

#include "AssetHandle.h"

namespace sif::asset {
    template<class T>
    AssetHandle<T>::AssetHandle() = default;

    template<class T>
    AssetHandle<T>::AssetHandle(std::weak_ptr<AssetRecord> record) : record_(std::move(record)) {

    }

    template<class T>
    std::weak_ptr<AssetRecord> AssetHandle<T>::record() const {
        return record_;
    }

    template<class T>
    bool AssetHandle<T>::ready() const {
        const auto r = record_.lock();
        return r && r->get_state() == AssetState::Ready && r->get_data();
    }

    template<class T>
    T * AssetHandle<T>::get() const {
        auto r = record_.lock();
        if (!r || r->get_state() != AssetState::Ready || !r->get_data()) {
            return nullptr;
        }
        // r->get_data() returns std::shared_ptr<void>
        auto typed = std::static_pointer_cast<T>(r->get_data());
        // raw pointer
        return typed.get();
    }

    template<class T>
    AssetHandle<T>::operator bool() const {
        return ready();
    }

    template<class T>
    intrnl::GUID AssetHandle<T>::guid() const {
        const auto r = record_.lock();
        return r ? r->get_meta().guid: intrnl::GUID{};
    }

    template<class T>
    void AssetHandle<T>::on_ready(std::function<void(T*)> callback) const {
        const auto r = record_.lock();
        if (!r) {
            // Nothing to wait for: the underlying asset no longer exists.
            callback(nullptr);
            return;
        }

        std::weak_ptr<AssetRecord> weak_record = record_;
        r->on_complete([weak_record, callback = std::move(callback)](AssetState final_state) {
            if (final_state != AssetState::Ready) {
                callback(nullptr);
                return;
            }
            const auto locked = weak_record.lock();
            if (!locked) {
                callback(nullptr);
                return;
            }
            const auto typed = std::static_pointer_cast<T>(locked->get_data());
            callback(typed.get());
        });
    }
}
