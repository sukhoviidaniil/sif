/***************************************************************
* Project:          Render_Engine
* File:             LogScope.h
*
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-10
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_LOGSCOPE_H
#define RENDER_ENGINE_LOGSCOPE_H

#ifdef _DEBUG
#define LOG_SCOPE() sif::diag::LogScope UNIQUE_NAME(__scope){}
#define UNIQUE_NAME(base) CONCAT(base, __COUNTER__)
#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a##b
#else
#define LOG_SCOPE()
#endif

namespace sif::diag {
    /**
     * @brief RAII helper for automatic log indentation management.
     *
     * Increases the logger indentation level on construction
     * and restores it on destruction.
     *
     * Intended to be used via the LOG_SCOPE() macro.
     */
    class LogScope {
    public:
        /**
         * @brief Constructs the log scope.
         * Increases the logger indentation depth.
         *
         */
        LogScope();

        /**
         * @brief Destroys the log scope and restores previous indentation.
         * Decreases the logger indentation depth.
         */
        ~LogScope();
    };
}

#endif //RENDER_ENGINE_LOGSCOPE_H