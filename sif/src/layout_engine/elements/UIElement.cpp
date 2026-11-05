/***************************************************************
 * Author:           Daniil Sukhovii
 * Email:            sukhovii.daniil@gmail.com
 * Created:          2026-01-11
 *
 * License:
 *       c. 2026 Daniil Sukhovii. All rights reserved.
 *       Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#include "sif/layout_engine/elements/UIElement.h"

#include <algorithm>
#include <numeric>

#include "sif/render/Layer.h"

namespace sif::ui {
namespace {
/**
 * @brief Best hit found so far by collect_hits.
 */
struct HitCandidate {
    std::vector<UIElement*> path;
    int layer = rnd::layer::Content;
    std::size_t sequence = 0;
    bool found = false;
};

/**
 * @brief Walks the tree in exactly the order UIElement::render paints
 * it and remembers the last-painted element under point.
 *
 * Mirroring the paint traversal (pre-order, children in
 * paint_order(), invisible subtrees skipped, layers inherited) is the
 * whole trick: "topmost under the cursor" then means precisely "drawn
 * last at that pixel", with no second notion of depth to keep in sync.
 */
void collect_hits(UIElement& e, const math::Vector2& point, const int inherited_layer,
                  const std::optional<intrnl::Rect>& inherited_clip, std::vector<UIElement*>& path, HitCandidate& best,
                  std::size_t& sequence) {
    if (!e.visible) {
        return;
    }
    // Outside a clipping ancestor nothing in this subtree is visible,
    // so nothing in it can be hit either.
    if (inherited_clip.has_value() && !inherited_clip->contains(point.x, point.y)) {
        return;
    }

    const int effective_layer = e.layer.value_or(inherited_layer);
    const std::size_t own_sequence = sequence++;
    path.push_back(&e);

    const intrnl::Rect& r = e.result.rect;
    if (e.hit_testable && r.width > 0.f && r.height > 0.f && r.contains(point.x, point.y)) {
        // Higher layer always wins; on the same layer the element
        // visited later was painted later, i.e. is on top.
        if (!best.found || effective_layer > best.layer ||
            (effective_layer == best.layer && own_sequence > best.sequence)) {
            best.path = path;
            best.layer = effective_layer;
            best.sequence = own_sequence;
            best.found = true;
        }
    }

    std::optional<intrnl::Rect> clip = inherited_clip;
    if (e.clip) {
        clip = clip.has_value() ? clip->intersection(r) : r;
    }

    for (const std::size_t i : e.paint_order()) {
        collect_hits(*e.children[i], point, effective_layer, clip, path, best, sequence);
    }

    path.pop_back();
}
} // namespace

void UIElement::add(std::unique_ptr<UIElement> child) { children.push_back(std::move(child)); }

math::Vector2 UIElement::measure(const math::Vector2& available) {
    math::Vector2 total_size{0, 0};

    for (auto& child : children) {
        auto child_size = child->measure(available);
        total_size.y += child_size.y + child->margin.y;
        total_size.x = std::max(total_size.x, child_size.x + child->margin.x);
    }

    total_size.x += padding.x * 2;
    total_size.y += padding.y * 2;

    total_size.x = std::clamp(total_size.x, min_size.x, max_size.x);
    total_size.y = std::clamp(total_size.y, min_size.y, max_size.y);

    return total_size;
}

void UIElement::layout(const intrnl::Rect r) {
    result.rect = r;

    float y_offset = r.y + padding.y;

    for (auto& child : children) {
        math::Vector2 child_size = child->resolve_size({r.width - padding.x * 2, r.height - padding.y * 2});

        intrnl::Rect child_rect{r.x + padding.x + child->margin.x, y_offset + child->margin.y, child_size.x,
                                child_size.y};

        child->layout(child_rect);

        y_offset += child_size.y + child->margin.y;
    }
}

void UIElement::render(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const {
    if (!visible) {
        return;
    }

    // Everything this subtree submits - own items and every
    // descendant's - lands on this layer and inside this clip unless a
    // descendant opens a scope of its own; the scopes restore the
    // parent's layer and clip on the way out.
    std::optional<rnd::LayerScope> layer_scope;
    if (layer.has_value()) {
        layer_scope.emplace(frame, *layer);
    }
    std::optional<rnd::ClipScope> clip_scope;
    if (clip) {
        clip_scope.emplace(frame, result.rect);
    }

    append_render_items(frame, ctx);
}

void UIElement::append_render_items(rnd::RenderFrame& frame, const rnd::FrameContext& ctx) const {
    // Through render(), not append_render_items(): that is what makes
    // a child's visible flag and layer apply to its whole subtree.
    for (const std::size_t i : paint_order()) {
        children[i]->render(frame, ctx);
    }
}

bool UIElement::on_ui_event(UIEvent& e) { return event_handler ? event_handler(e) : false; }

bool UIElement::can_focus() const { return focusable; }

std::vector<std::size_t> UIElement::paint_order() const {
    std::vector<std::size_t> order(children.size());
    std::iota(order.begin(), order.end(), std::size_t{0});

    // Stable, so equal z_index keeps document order and a tree that
    // never sets z_index paints exactly as it did before z_index
    // existed. The vector is tiny (one entry per direct child), which
    // is negligible next to the render items allocated every frame.
    std::stable_sort(order.begin(), order.end(), [this](const std::size_t a, const std::size_t b) {
        return children[a]->z_index < children[b]->z_index;
    });

    return order;
}

std::vector<UIElement*> UIElement::hit_path(const math::Vector2& point) {
    HitCandidate best;
    std::vector<UIElement*> path;
    std::size_t sequence = 0;

    collect_hits(*this, point, rnd::layer::Content, std::nullopt, path, best, sequence);

    return best.found ? best.path : std::vector<UIElement*>{};
}

void UIElement::update(const float dt) {
    for (auto& c : children) {
        c->update(dt);
    }
}

UIElement* UIElement::find_if(const std::function<bool(const UIElement&)>& predicate) {
    if (predicate(*this)) {
        return this;
    }
    for (auto& c : children) {
        if (UIElement* found = c->find_if(predicate)) {
            return found;
        }
    }
    return nullptr;
}

math::Vector2 UIElement::resolve_size(const math::Vector2& available) const {
    math::Vector2 out;

    out.x = width.resolve(available.x);
    out.y = height.resolve(available.y);

    out.x = std::clamp(out.x, min_size.x, max_size.x);
    out.y = std::clamp(out.y, min_size.y, max_size.y);

    return out;
}
} // namespace sif::ui
