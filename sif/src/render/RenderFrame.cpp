/***************************************************************
 * Author:          Daniil Sukhovii
 * Email:           sukhovii.daniil@gmail.com
 * Created:         2026-11-05
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#include "sif/render/RenderFrame.h"

#include <algorithm>

namespace sif::rnd {
void RenderFrame::submit(std::unique_ptr<RenderNode> item) {
    if (item == nullptr) {
        return;
    }
    item->layer = current_layer_;
    item->clip = current_clip_;
    temp_items.push_back(std::move(item));
}

int RenderFrame::current_layer() const { return current_layer_; }

const std::optional<intrnl::Rect>& RenderFrame::current_clip() const { return current_clip_; }

std::vector<const RenderNode*> RenderFrame::draw_order() const {
    std::vector<const RenderNode*> order;
    order.reserve(constant_items.size() + temp_items.size());

    for (const auto& item : constant_items) {
        order.push_back(item.get());
    }
    for (const auto& item : temp_items) {
        order.push_back(item.get());
    }

    // Stable, not plain sort: inside one layer the submission order is
    // the paint order the layout tree asked for, and a plain sort would
    // be free to shuffle it.
    std::stable_sort(order.begin(), order.end(),
                     [](const RenderNode* a, const RenderNode* b) { return a->layer < b->layer; });

    return order;
}

LayerScope::LayerScope(RenderFrame& frame, const int layer) : frame_(frame), previous_(frame.current_layer_) {
    frame_.current_layer_ = layer;
}

LayerScope::~LayerScope() { frame_.current_layer_ = previous_; }

ClipScope::ClipScope(RenderFrame& frame, const intrnl::Rect& clip) : frame_(frame), previous_(frame.current_clip_) {
    frame_.current_clip_ = previous_.has_value() ? previous_->intersection(clip) : clip;
}

ClipScope::~ClipScope() { frame_.current_clip_ = previous_; }
} // namespace sif::rnd
