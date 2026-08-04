/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-11-19
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/

#include "sif/diagnostics/Logger.h"

#include <algorithm>
#include <atomic>
#include <sstream>

namespace sif::diag {
    namespace {
        /// Hands out 0, 1, 2, ... to threads in the order they first log.
        std::atomic<unsigned int> next_thread_ordinal{0};
    }

    Logger::Logger() : logfile_("debug.log") {
        logfile_.flush();
    }

    Logger & Logger::instance() {
        static Logger inst;
        return inst;
    }

    unsigned int & Logger::depth() {
        // Per-thread: an indentation scope belongs to the call stack
        // that opened it, not to the process.
        static thread_local unsigned int value = 0;
        return value;
    }

    const std::string & Logger::thread_tag() {
        static thread_local const std::string tag = [] {
            const unsigned int ordinal = next_thread_ordinal.fetch_add(1, std::memory_order_relaxed);
            // The first thread to log (in practice the main one) stays
            // unmarked, so a single-threaded log looks exactly as before.
            return ordinal == 0 ? std::string{} : "[t" + std::to_string(ordinal) + "] ";
        }();
        return tag;
    }

    void Logger::write(const std::string &message) {
        // Formatting happens outside the lock; only the stream needs it.
        const std::string indented = indent(message);

        std::lock_guard lock(mtx_);
        // One insertion, not several: with `<< a << b << std::endl` a
        // second thread can slip between the parts even under a lock
        // held elsewhere, and unlocked it corrupts the stream buffer.
        logfile_ << indented + '\n';
        logfile_.flush();
    }

    void Logger::add_depth() {
        unsigned int& d = depth();
        if (d < max_depth) {
            ++d;
        }
    }

    void Logger::lower_depth() {
        unsigned int& d = depth();
        if (d > 0) {
            --d;
        }
    }

    std::string Logger::indent(const std::string &message) const {
        std::string total_indent;
        const unsigned int levels = std::min(depth(), max_depth);
        total_indent.reserve(levels * indent_str_.size());
        for (unsigned int i = 0; i < levels; ++i) {
            total_indent += indent_str_;
        }

        const std::string prefix = total_indent + thread_tag();

        std::ostringstream ss;
        std::istringstream msg_stream(message);
        std::string line;
        bool first = true;
        while (std::getline(msg_stream, line)) {
            if (!first) {
                ss << '\n';
            }
            ss << prefix << line;
            first = false;
        }

        return ss.str();
    }

    void Logger::print_exception(const std::exception &e, const int level) {
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
            // A nested object that is not a std::exception carries no
            // message we can print; the outer frames are already logged.
        }
    }
}