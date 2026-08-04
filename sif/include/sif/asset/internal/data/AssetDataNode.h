/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-14
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_METAASSETDATA_H
#define RENDER_ENGINE_METAASSETDATA_H

#include <string>

#include "AssetMetaData.h"

namespace sif::asset::data {
    struct  AssetDataNode {
        std::string source;
        AssetMetaData meta;
    };
}

#endif //RENDER_ENGINE_METAASSETDATA_H