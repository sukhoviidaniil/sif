/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-10
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#include "sif/diagnostics/LogScope.h"
#include "sif/diagnostics/Logger.h"

namespace sif::diag{
    LogScope::LogScope() {
        Logger::instance().add_depth();
    }

    LogScope::~LogScope() {
        Logger::instance().lower_depth();
    }
}
