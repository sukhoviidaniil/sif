/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_SFML_SFMLASSETLOADERS_H
#define RENDER_ENGINE_SFML_SFMLASSETLOADERS_H

#include "sif/asset/AssetRegistry.h"
#include "sif/asset/internal/IAssetLoader.h"

namespace sif::sfml {
    /**
     * @brief Base of every loader in this backend: all of them run on the
     * main thread.
     *
     * Not a policy choice - a constraint of the libraries underneath.
     * sf::Texture and sf::Font need an OpenGL context and take SFML's
     * TransientContextLock, which races with the main thread's rendering;
     * sf::SoundBuffer goes through OpenAL, which races with its own mixer
     * thread. ThreadSanitizer catches both inside the libraries, and what
     * a player sees is heap corruption: "double free or corruption",
     * "malloc(): unaligned tcache chunk detected", or a segfault with a
     * stack that has nothing to do with assets. Pressing a key during
     * start-up is enough, because that plays a sound while the rest of
     * the assets are still being decoded.
     *
     * The registry hands these to AssetRegistry::pump() instead of a
     * worker thread. The parallelism is lost for this backend; the
     * headless backend keeps it, because sf::Image and sf::InputSoundFile
     * are pure CPU decoding and touch no device.
     */
    class SFML_MainThreadLoader : public asset::IAssetLoader {
        [[nodiscard]] bool runs_on_main_thread() const final { return true; }
    };

    /**
     * @brief IAssetLoader implementations, one per AssetType.
     *
     * Each of them does exactly three things: read the *.asset.json
     * node for the asset, build the matching SFML-backed object, and
     * hand it to the record with set_data_if_current(). They never
     * touch AssetState - that transition belongs to
     * IAssetLoader::try_load, which also enforces the promised load
     * time.
     */

    class SFML_Font_AssetLoader final : public SFML_MainThreadLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_SpriteSingle_AssetLoader final : public SFML_MainThreadLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_SpriteAtlas_AssetLoader final : public SFML_MainThreadLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_SpriteGrid_AssetLoader final : public SFML_MainThreadLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_PrimitiveAnimation_AssetLoader final : public SFML_MainThreadLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    class SFML_Sound_AssetLoader final : public SFML_MainThreadLoader {
        void load_asset(asset::AssetRecord& record, const std::string& asset_dir, uint64_t attempt_token) override;
    };

    /**
     * @brief Registers one loader per AssetType with the registry.
     *
     * Called once during start-up; without it every request() would
     * fail with "There is no loader for: ...", which is exactly what
     * used to happen for everything except fonts.
     */
    void register_sfml_asset_loaders(asset::AssetRegistry& registry);
}

#endif //RENDER_ENGINE_SFML_SFMLASSETLOADERS_H
