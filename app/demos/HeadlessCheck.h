/***************************************************************
 * Project:       Render_Engine (demos)
 * File:          HeadlessCheck.h
 *
 * Author:        Sukhovii Daniil
 * Email:         sukhovii.daniil@gmail.com
 * Created:       2026-08-03
 *
 * License:
 *      c. 2026 Daniil Sukhovii. All rights reserved.
 *      Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_HEADLESSCHECK_H
#define RENDER_ENGINE_HEADLESSCHECK_H

namespace app::demo {
    /**
     * @brief Loads and verifies every demo asset without a window.
     *
     * @return 0 if every check passed, 1 otherwise (so CI can use the
     * process exit code directly).
     */
    int run_headless_check();
}

#endif //RENDER_ENGINE_HEADLESSCHECK_H
