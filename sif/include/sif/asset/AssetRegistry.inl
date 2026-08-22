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

#include <stdexcept>

#include "AssetRegistry.h"

namespace sif::asset {
    template<typename T>
    AssetHandle<T> AssetRegistry::get(intrnl::GUID id) const {
        std::lock_guard lock(mtx_);
        const auto rec = by_guid_.find(id);
        if (rec == by_guid_.end()) {
            // A bare `throw;` outside of a catch block calls
            // std::terminate(), so an unknown GUID used to kill the
            // process instead of reporting anything.
            throw std::runtime_error("AssetRegistry::get - no asset registered with GUID " + id.string());
        }
        return AssetHandle<T>(std::weak_ptr<asset::AssetRecord>(rec->second));
    }
} // namespace sif::asset
