/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-01-14
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETTYPE_H
#define RENDER_ENGINE_ASSETTYPE_H

#include <string>

namespace sif::asset {
    enum struct AssetType {
        Undefined,
        Font,
        SpriteSingle,
        SpriteAtlas,
        SpriteGrid,
        PrimitiveAnimation,
        Sound
    };

    inline AssetType from_string(const std::string& s) {
        if (s == "Font")               return AssetType::Font;
        if (s == "SpriteSingle")       return AssetType::SpriteSingle;
        if (s == "SpriteAtlas")        return AssetType::SpriteAtlas;
        if (s == "SpriteGrid")         return AssetType::SpriteGrid;
        if (s == "PrimitiveAnimation") return AssetType::PrimitiveAnimation;
        if (s == "Sound")              return AssetType::Sound;
        return AssetType::Undefined;
    }

    inline std::string to_string(const AssetType& t){
        switch (t) {
            case AssetType::Font :               return "Font";
            case AssetType::SpriteSingle :       return "SpriteSingle";
            case AssetType::SpriteAtlas :        return "SpriteAtlas";
            case AssetType::SpriteGrid :         return "SpriteGrid";
            case AssetType::PrimitiveAnimation : return "PrimitiveAnimation";
            case AssetType::Sound :              return "Sound";
            default: return "Undefined";
        }
    }
}

#endif //RENDER_ENGINE_ASSETTYPE_H