/***************************************************************
* Author:           Daniil Sukhovii
* Email:            sukhovii.daniil@gmail.com
* Created:          2025-12-10
*
* License:
*       c. 2026 Daniil Sukhovii. All rights reserved.
*       Unauthorized use, reproduction, or distribution is prohibited.
***************************************************************/
#ifndef RENDER_ENGINE_LOGGER_H
#define RENDER_ENGINE_LOGGER_H

#include <fstream>
#include <mutex>
#include <string>

#ifdef _DEBUG
#define LOG(x) sif::diag::Logger::instance().write(x)
#define LOG_ADD_DEPTH() sif::diag::Logger::instance().add_depth();
#define LOG_LOWER_DEPTH() sif::diag::Logger::instance().lower_depth();
#else
#define LOG(x)
#define LOG_ADD_DEPTH()
#define LOG_LOWER_DEPTH()
#endif


namespace sif::diag {
    /**
     * @brief Simple singleton logger for debug output.
     *
     * Writes formatted messages to a log file and supports
     * indentation-based depth tracking for structured logs.
     *
     * @par Thread safety
     * Every LOG() in this project can be reached from a background
     * asset-loader thread as well as from the main thread, so both
     * pieces of shared state are protected:
     *
     *  - the output stream is guarded by a mutex, and one message is
     *    written with a single stream operation. std::ofstream is not
     *    safe for concurrent writes: two threads inside xsputn() race
     *    on the same put area, which shows up first as interleaved
     *    half-lines in debug.log and eventually as a segfault.
     *  - the indentation depth is thread_local. Depth describes the
     *    call nesting of *one* thread, so a LOG_SCOPE() opened on the
     *    main thread must not indent (or, worse, be closed by) a loader
     *    thread. Sharing one counter made unbalanced add/lower pairs
     *    across threads unavoidable.
     *
     * Messages coming from a thread other than the first one to log are
     * prefixed with a short thread tag, so an interleaved log can still
     * be read.
     */
    class Logger {
    public:
        /**
         * @brief Returns the global logger instance.
         */
        static Logger& instance();

        /**
         * @brief Writes a message to the log.
         *
         * The message is automatically indented based on
         * the current depth.
         *
         * @param message Text to write.
         */
        void write(const std::string& message);

        /**

         */
        void print_exception(const std::exception& e, int level = 0);


        /**
         * @brief Increases the indentation depth.
         *
         * Typically used to represent entering a nested scope.
         */
        void add_depth();

        /**
         * @brief Decreases the indentation depth of the calling thread.
         *
         * Saturates at 0. The previous implementation decremented an
         * unsigned counter unconditionally despite this very note, so
         * one unbalanced scope wrapped the depth to ~4e9 and the next
         * message tried to build a four-billion-level indent string.
         */
        void lower_depth();
    private:
        /**
         * @brief Constructs the logger and opens the log file.
         */
        Logger();

        /**
         * @brief Indentation depth of the calling thread.
         *
         * A function-local thread_local rather than a data member:
         * depth is per-thread state that merely happens to be reported
         * through a shared object.
         */
        static unsigned int& depth();

        /// @brief Short marker identifying the calling thread ("" for the first one).
        static const std::string& thread_tag();

        mutable std::mutex mtx_;    ///< Guards logfile_
        std::ofstream logfile_;     ///< Output log file stream
        std::string indent_str_ = "  "; ///< String used for one indentation level

        /// @brief Upper bound on indentation, so a runaway depth cannot
        /// turn one log line into a gigabyte-sized string.
        static constexpr unsigned int max_depth = 32;

        /**
         * @brief Applies indentation to a message.
         *
         * @param message Input message.
         * @return Indented message.
         */
        std::string indent(const std::string& message) const;
    };
}




#endif //RENDER_ENGINE_LOGGER_H