/***************************************************************
 * Project:       render-engine
 * File:          ViewContext.h
 *
 * Author:        Sukhovii Daniil
 * Created:       2025-12-26
 * Modified:      []
 *
 * Description:   []
 *
 * Contact:
 *   Email:       sukhovii.daniil@gmail.com
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