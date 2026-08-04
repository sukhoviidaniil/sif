/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-10
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/diagnostics/Logger.h"
#include <sstream>

namespace sif::diag {
    Logger::Logger() : logfile_("debug.log"), depth_(0), indent_str_("  ") {
        logfile_.flush();
    }

    Logger & Logger::instance() {
        static Logger inst;
        return inst;
    }

    void Logger::write(const std::string &message) {
        std::string indented = indent(message);
        logfile_ << indented << std::endl;
    }

    void Logger::add_depth() {
        depth_++;
    }

    void Logger::lower_depth() {
        depth_--;
    }

    std::string Logger::indent(const std::string &message) const {
        std::stringstream ss;
        std::istringstream msgStream(message);
        std::string line;
        std::string total_indent;
        for (unsigned int i = 0; i < depth_; ++i)
            total_indent += indent_str_;

        while (std::getline(msgStream, line)) {
            ss << total_indent << line << '\n';
        }
        std::string result = ss.str();
        if (!result.empty() && result.back() == '\n') result.pop_back();
        return result;
    }

    void Logger::print_exception(const std::exception &e, int level) {
        add_depth();
        write(e.what());
        lower_depth();

        try {
            std::rethrow_if_nested(e);
        }
        catch (const std::exception& nested) {
            print_exception(nested, level + 1);
        }
        catch (...) {

        }
    }
}


