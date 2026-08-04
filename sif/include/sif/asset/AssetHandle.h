/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-13
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETHANDLE_H
#define RENDER_ENGINE_ASSETHANDLE_H
#include <functional>
#include <memory>

#include "internal/AssetRecord.h"

namespace sif::asset {
    /**
     * @brief Lightweight, typed reference to an asset managed by AssetRegistry.
     */
    template<class T>
    class AssetHandle {
    public:
        AssetHandle();
        explicit AssetHandle(std::weak_ptr<AssetRecord> record);

        /**
         * @brief Returns the record this handle refers to.
         *
         * Exposed so a typed handle can be re-wrapped as a typeless
         * one (AssetHandle<void>) when it is handed to a render item,
         * without the registry having to be consulted a second time.
         */
        [[nodiscard]] std::weak_ptr<AssetRecord> record() const;

        [[nodiscard]] bool ready() const;

        T* get() const;
        explicit operator bool() const;
        [[nodiscard]] intrnl::GUID guid() const;

        /**
         * @brief Registers a callback for when the asset finishes loading.
         *
         * Avoids having to poll ready()/get() every frame: callback is
         * invoked exactly once, with the loaded object (or nullptr if
         * the load failed, or the underlying asset record no longer
         * exists).
         *
         * If the asset has already finished loading (successfully or
         * not) by the time this is called, callback fires immediately,
         * synchronously, on the calling thread. Otherwise it fires
         * later, from whichever thread completes the load — typically
         * a background loader thread, not the main/render thread — so
         * keep the callback body short, and avoid touching
         * non-thread-safe state directly from it (e.g. hop back to the
         * main thread via your own event queue if needed).
         *
         * @param callback Invoked once with the loaded T*, or nullptr on failure.
         */
        void on_ready(std::function<void(T*)> callback) const;

    private:
        std::weak_ptr<AssetRecord> record_;
    };
}

#include "AssetHandle.inl"

#endif //RENDER_ENGINE_ASSETHANDLE_H