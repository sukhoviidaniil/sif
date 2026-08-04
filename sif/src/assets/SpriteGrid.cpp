/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2026-07-06
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/asset/internal/SpriteGrid.h"

#include <stdexcept>

namespace sif::asset {
    SpriteGrid::SpriteGrid(math::Matrix<intrnl::Rect> cells)
        : cells_(std::move(cells)) {
    }

    SpriteGrid::~SpriteGrid() = default;

    SpriteGrid::SpriteGrid(SpriteGrid&&) noexcept = default;

    SpriteGrid& SpriteGrid::operator=(SpriteGrid&&) noexcept = default;

    size_t SpriteGrid::rows() const {
        return cells_.rows();
    }

    size_t SpriteGrid::cols() const {
        return cells_.cols();
    }

    const intrnl::Rect& SpriteGrid::cell(size_t row, size_t col) const {
        if (row >= cells_.rows() || col >= cells_.cols()) {
            throw std::out_of_range("SpriteGrid::cell - (row, col) out of range");
        }
        return cells_(row, col);
    }

    const intrnl::Rect& SpriteGrid::cell(intrnl::RecordID id) const {
        const size_t col_count = cells_.cols();
        if (col_count == 0) {
            throw std::out_of_range("SpriteGrid::cell - grid has no columns");
        }
        const size_t row = id.id / col_count;
        const size_t col = id.id % col_count;
        return cell(row, col);
    }
}
