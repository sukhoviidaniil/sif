/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-14
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_ASSETSTATE_H
#define RENDER_ENGINE_ASSETSTATE_H

namespace sif::asset {
    /**
     * @brief Lifecycle state of an asset record.
     *
     * NotRequested -> Queued -> Loading -> Ready
     *                                   \-> Failed
     */
    enum class AssetState { NotRequested, Queued, Loading, Ready, Failed };
} // namespace sif::asset

#endif // RENDER_ENGINE_ASSETSTATE_H