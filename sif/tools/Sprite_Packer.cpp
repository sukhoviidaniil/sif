/***************************************************************

 * Author:        Sukhovii Daniil
 * Email:       sukhovii.daniil@gmail.com
 * Created:       2026-08-10
 *
 * Disclaimer:
 *   This file is part of render-engine.
 *   Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "SFML/Graphics/Image.hpp"

#include "json.hpp"

#include "sif/io/from_JSON.h"

/**
 * @file
 *
 * Packs per-frame artwork into sprite strips and writes the matching
 * *.asset.json descriptors.
 *
 * @par Why this belongs in the engine
 * asset::PrimitiveAnimation addresses frames as rectangles *inside one
 * texture*: one file, one GPU texture, one draw call per entity. Almost every
 * free art pack, on the other hand, ships one PNG per frame. Bridging the two
 * means maintaining a strip image and a list of pixel rectangles that must
 * agree with it exactly - which is precisely the kind of thing that should be
 * generated from one source of truth rather than kept in step by hand.
 *
 * Every game built on sif that uses per-frame art hits this, so the tool lives
 * here next to Asset_GUID_Assignment instead of being re-invented as a script
 * in each project.
 *
 * @par What it deliberately does not do
 * It does not build the registry (Asset_GUID_Assignment does that), and it
 * does not touch assets that need no processing. A tile or a sound effect is
 * already loadable as it is, so its descriptor is a hand-written file that
 * names the original - copying bytes to say what the JSON already says would
 * be duplication for its own sake. Single-frame entries ("stills") are handled
 * the same way: the descriptor points at the existing image and declares one
 * frame covering it, so nothing is copied.
 *
 * @par Determinism
 * Frames are packed in the declared order and GUIDs are assigned from a fixed
 * base, so re-running the tool on a clean checkout reproduces the output byte
 * for byte. That matters: a packer that shuffles its output would make the
 * generated files churn in version control on every build.
 *
 * @par Usage
 * @code
 * sif_sprite_packer <asset_dir> <pack_file.json>
 * @endcode
 *
 * The pack file (all paths relative to `asset_dir`):
 * @code{.json}
 * {
 *     "output_dir": "graphics/sprites",
 *     "guid_base": 3000000,
 *     "animations": [
 *         {
 *             "asset_name": "player_walk_down",
 *             "frame_duration_seconds": 0.12,
 *             "loop": true,
 *             "align": "bottom",
 *             "frames": ["art/walk1.png", "art/idle.png", "art/walk2.png"]
 *         }
 *     ],
 *     "stills": [
 *         { "asset_name": "player_idle_down", "source": "art/idle.png" }
 *     ]
 * }
 * @endcode
 */
namespace {
    namespace fs = std::filesystem;

    /// How a frame smaller than the cell is placed inside it.
    enum class Align {
        Bottom, ///< Feet on the floor - the right choice for characters.
        Center  ///< The right choice for bombs, fire and pick-ups.
    };

    Align align_from_string(const std::string& value, const std::string& asset_name) {
        if (value == "bottom") return Align::Bottom;
        if (value == "center") return Align::Center;
        throw std::runtime_error(
            "sprite pack: '" + asset_name + "' has align \"" + value +
            "\", expected \"bottom\" or \"center\"");
    }

    sf::Image load_image(const fs::path& path) {
        sf::Image image;
        if (!image.loadFromFile(path.string())) {
            throw std::runtime_error("sprite pack: cannot read image '" + path.string() + "'");
        }
        return image;
    }

    /// Relative, forward-slash path, as an asset descriptor's `source` needs.
    std::string relative_source(const fs::path& file, const fs::path& asset_dir) {
        return fs::relative(file, asset_dir).generic_string();
    }

    /**
     * @brief Packs frames left to right into one strip of uniform cells.
     *
     * The cell is as large as the biggest frame; smaller frames are centred
     * horizontally and aligned per `align` vertically, so a walk cycle whose
     * frames differ by a pixel or two does not bob up and down.
     *
     * @return The cell size.
     */
    sf::Vector2u pack_strip(const std::vector<sf::Image>& frames, const Align align,
                            const fs::path& output_png) {
        unsigned int cell_w = 0;
        unsigned int cell_h = 0;
        for (const sf::Image& frame : frames) {
            cell_w = std::max(cell_w, frame.getSize().x);
            cell_h = std::max(cell_h, frame.getSize().y);
        }

        sf::Image sheet;
        sheet.create(cell_w * static_cast<unsigned int>(frames.size()), cell_h,
                     sf::Color::Transparent);

        for (std::size_t i = 0; i < frames.size(); ++i) {
            const sf::Vector2u size = frames[i].getSize();
            const unsigned int x =
                static_cast<unsigned int>(i) * cell_w + (cell_w - size.x) / 2;
            const unsigned int y =
                align == Align::Bottom ? cell_h - size.y : (cell_h - size.y) / 2;

            // applyAlpha = false: the destination is fully transparent, and
            // blending onto it would multiply the colour by the source alpha
            // and leave dark fringes around every soft edge. A raw copy is
            // both correct and what "packing" means.
            sheet.copy(frames[i], x, y, sf::IntRect(), false);
        }

        fs::create_directories(output_png.parent_path());
        if (!sheet.saveToFile(output_png.string())) {
            throw std::runtime_error("sprite pack: cannot write '" + output_png.string() + "'");
        }

        return {cell_w, cell_h};
    }

    nlohmann::json frame_rects(const sf::Vector2u cell, const std::size_t count) {
        nlohmann::json frames = nlohmann::json::array();
        for (std::size_t i = 0; i < count; ++i) {
            frames.push_back({
                {"x", static_cast<float>(i) * static_cast<float>(cell.x)},
                {"y", 0.f},
                {"width", static_cast<float>(cell.x)},
                {"height", static_cast<float>(cell.y)}
            });
        }
        return frames;
    }

    void write_descriptor(const fs::path& path, const std::string& asset_name,
                          const std::string& guid, const std::string& source,
                          const float frame_duration, const bool loop,
                          const nlohmann::json& frames) {
        nlohmann::json descriptor = nlohmann::json::object();
        descriptor["type"] = "PrimitiveAnimation";
        descriptor["asset_name"] = asset_name;
        descriptor["guid"] = guid;
        descriptor["source"] = source;
        descriptor["expected_load_time_seconds"] = 5.0;
        descriptor["frame_duration_seconds"] = frame_duration;
        descriptor["loop"] = loop;
        descriptor["frames"] = frames;

        // The engine's own writer: creates directories, writes to a temp file
        // and renames it into place, so an interrupted run cannot leave a
        // half-written descriptor behind.
        sif::io::write_json_file(path, descriptor);
    }

    /// Explicit "guid" if the entry has one, otherwise base + index.
    std::string guid_for(const nlohmann::json& entry, const std::uint64_t base,
                         const std::size_t index) {
        if (entry.contains("guid") && !entry.at("guid").is_null()) {
            return entry.at("guid").get<std::string>();
        }
        return std::to_string(base + index + 1);
    }

    std::string require_string(const nlohmann::json& entry, const std::string& key,
                               const std::string& what) {
        if (!entry.contains(key) || !entry.at(key).is_string()) {
            throw std::runtime_error("sprite pack: " + what + " needs a string \"" + key + "\"");
        }
        return entry.at(key).get<std::string>();
    }
}

int main(const int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr
            << "Usage: " << argv[0] << " <asset_dir> <pack_file>\n"
            << "  asset_dir  root every path in the pack file is relative to\n"
            << "  pack_file  JSON describing the animations to pack\n\n"
            << "Packs per-frame artwork into sprite strips and writes the matching\n"
            << "*.asset.json descriptors. Run Asset_GUID_Assignment afterwards to\n"
            << "rebuild the registry.\n";
        return 1;
    }

    const fs::path asset_dir = argv[1];
    const fs::path pack_file = argv[2];

    try {
        if (!fs::is_directory(asset_dir)) {
            throw std::runtime_error("sprite pack: asset directory does not exist: " + asset_dir.string());
        }

        const nlohmann::json pack = sif::io::get_json_data(pack_file);
        if (!pack.is_object()) {
            throw std::runtime_error("sprite pack: '" + pack_file.string() + "' must contain an object");
        }

        const fs::path output_dir =
            asset_dir / sif::io::get_checked<std::string>(pack, "output_dir", "sprites");
        const auto guid_base = sif::io::get_checked<std::uint64_t>(pack, "guid_base", 3000000);

        std::size_t index = 0;
        std::size_t packed = 0;
        std::size_t stills = 0;

        // ---------- Multi-frame animations: packed into strips ----------
        if (pack.contains("animations")) {
            if (!pack.at("animations").is_array()) {
                throw std::runtime_error("sprite pack: 'animations' must be an array");
            }

            for (const nlohmann::json& entry : pack.at("animations")) {
                const std::string asset_name = require_string(entry, "asset_name", "an animation");

                if (!entry.contains("frames") || !entry.at("frames").is_array() ||
                    entry.at("frames").empty()) {
                    throw std::runtime_error(
                        "sprite pack: animation '" + asset_name + "' needs a non-empty \"frames\" array");
                }

                std::vector<sf::Image> frames;
                frames.reserve(entry.at("frames").size());
                for (const auto& frame : entry.at("frames")) {
                    frames.push_back(load_image(asset_dir / frame.get<std::string>()));
                }

                const Align align = align_from_string(
                    sif::io::get_checked<std::string>(entry, "align", "bottom"), asset_name);

                const fs::path png = output_dir / (asset_name + ".png");
                const sf::Vector2u cell = pack_strip(frames, align, png);

                write_descriptor(
                    output_dir / (asset_name + ".asset.json"),
                    asset_name,
                    guid_for(entry, guid_base, index),
                    relative_source(png, asset_dir),
                    sif::io::get_checked<float>(entry, "frame_duration_seconds", 0.1f),
                    sif::io::get_checked<bool>(entry, "loop", true),
                    frame_rects(cell, frames.size()));

                std::cout << "  packed   " << asset_name << "  ("
                          << frames.size() << " frames of " << cell.x << "x" << cell.y << ")\n";
                ++index;
                ++packed;
            }
        }

        // ---------- Single frames: described in place, never copied ----------
        if (pack.contains("stills")) {
            if (!pack.at("stills").is_array()) {
                throw std::runtime_error("sprite pack: 'stills' must be an array");
            }

            for (const nlohmann::json& entry : pack.at("stills")) {
                const std::string asset_name = require_string(entry, "asset_name", "a still");
                const std::string source = require_string(entry, "source", "still '" + asset_name + "'");

                const fs::path image_path = asset_dir / source;
                const sf::Image image = load_image(image_path);

                nlohmann::json frames = nlohmann::json::array();
                frames.push_back({
                    {"x", 0.f}, {"y", 0.f},
                    {"width", static_cast<float>(image.getSize().x)},
                    {"height", static_cast<float>(image.getSize().y)}
                });

                write_descriptor(
                    output_dir / (asset_name + ".asset.json"),
                    asset_name,
                    guid_for(entry, guid_base, index),
                    source, // the original file, untouched
                    sif::io::get_checked<float>(entry, "frame_duration_seconds", 1.f),
                    sif::io::get_checked<bool>(entry, "loop", true),
                    frames);

                std::cout << "  still    " << asset_name << "  (" << source << ")\n";
                ++index;
                ++stills;
            }
        }

        if (packed == 0 && stills == 0) {
            std::cout << "sprite pack: nothing to do - '" << pack_file.string()
                      << "' declares no animations and no stills\n";
        } else {
            std::cout << "sprite pack: " << packed << " strip(s) and " << stills
                      << " still(s) written to " << output_dir.string() << '\n';
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Sprite_Packer failed: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Sprite_Packer failed: unknown error\n";
        return 1;
    }
}
