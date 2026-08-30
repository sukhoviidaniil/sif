/***************************************************************
 * Author:        Sukhovii Daniil
 * Email:       sukhovii.daniil@gmail.com
 * Created:       2025-12-26
 *
 * Disclaimer:
 *   This file is part of render-engine.
 *   Unauthorized use, reproduction, or distribution is prohibited.
 ***************************************************************/
#ifndef RENDER_ENGINE_VIEWCONTEXT_H
#define RENDER_ENGINE_VIEWCONTEXT_H

namespace sif::rnd {
    struct FrameContext {
        const bool redrawing;

        explicit FrameContext(const bool redraw) : redrawing(redraw) {}
    };
} // namespace sif::rnd

#endif // RENDER_ENGINE_VIEWCONTEXT_H