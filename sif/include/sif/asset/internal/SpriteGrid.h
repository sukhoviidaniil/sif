/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_ASSETS_SPRITEGRID_H
#define RENDER_ENGINE_ASSETS_SPRITEGRID_H

#include <cstddef>

#include "sif/math/Matrix.h"
#include "sif/math/Vector2.h"
#include "sif/internal/Rect.h"
#include "sif/internal/RecordID.h"

namespace sif::asset {
    /**
     * @brief One shared texture cut into a uniform rows x cols grid of
     * sub-sprites (AssetType::SpriteGrid) - "a matrix of sprites".
     *
     * The cell geometry is backend-agnostic and stored directly as a
     * math::Matrix<intrnl::Rect>, tiled by the loader from the grid's
     * row/column count and cell size (see data::SpriteGridNode). The
     * actual pixel data lives in a backend-specific subclass that only
     * needs to implement texture_size() (e.g. sif::sfml::SFMLSpriteGrid,
     * built by SFML_SpriteGrid_AssetLoader).
     */
    class SpriteGrid {
    public:
        SpriteGrid(const SpriteGrid&) = delete;
        SpriteGrid& operator=(const SpriteGrid&) = delete;

        explicit SpriteGrid(math::Matrix<intrnl::Rect> cells);
        virtual ~SpriteGrid();
        SpriteGrid(SpriteGrid&&) noexcept;
        SpriteGrid& operator=(SpriteGrid&&) noexcept;

        /**
         * @brief Number of rows in the grid.
         */
        [[nodiscard]] size_t rows() const;

        /**
         * @brief Number of columns in the grid.
         */
        [[nodiscard]] size_t cols() const;

        /**
         * @brief Returns the pixel rect of the cell at (row, col).
         *
         * Throws std::out_of_range if row/col are out of bounds.
         */
        [[nodiscard]] const intrnl::Rect& cell(size_t row, size_t col) const;

        /**
         * @brief Returns the pixel rect of the cell at a flattened,
         * row-major index (row * cols() + col).
         *
         * Throws std::out_of_range if id is out of bounds.
         */
        [[nodiscard]] const intrnl::Rect& cell(intrnl::RecordID id) const;

        /**
         * @brief Returns the pixel size of the whole shared texture.
         */
        [[nodiscard]] virtual math::Vector2 texture_size() const = 0;

    private:
        math::Matrix<intrnl::Rect> cells_;
    };
}

#endif //RENDER_ENGINE_ASSETS_SPRITEGRID_H
