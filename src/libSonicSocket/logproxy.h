#ifndef SONICSOCKET_LOGGER_H
#define SONICSOCKET_LOGGER_H

#include <string>
#include <mutex>
#include <vector>

#include "libSonicSocket/jw_util/signal.h"
#include "libSonicSocket/jw_util/thread.h"

#include "libSonicSocket/config/SS_LOGPROXY_FORCE_LOG_INTERVAL.h"

namespace sonic_socket
{

using std::to_string;
inline std::string to_string(const char *str) {return str;}
inline std::string to_string(const std::string &str) {return str;}

class LogProxy
{
public:
    enum class LogLevel {Debug, Notice, Warning, Error, Fatal};

    template <LogLevel level>
    static constexpr bool is_tracking_level()
    {
#ifndef NDEBUG
        (void) level;
        return true;
#else
        return level != LogLevel::Debug;
#endif
    }

    template <LogLevel level, typename... ArgTypes>
    void push_event(const ArgTypes & ... args)
    {
        if (!is_tracking_level<level>()) {return;}

        std::string str;
        concat_str(str, get_level_str(level), ": ", args...);

        std::lock_guard<std::mutex> lock(pending_logs_mutex);
        (void) lock;
        pending_logs.emplace_back(level, std::move(str));
    }

    static constexpr const char *get_level_str(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Notice:
            return "NOTICE";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Fatal:
            return "FATAL";
        default:
            assert(false);
            return "";
        }
    }

    void tick()
    {
        if (last_log_event < SS_LOGPROXY_FORCE_LOG_INTERVAL)
        {
            std::unique_lock<std::mutex> lock(pending_logs_mutex, std::try_to_lock);
            if (lock.owns_lock())
            {
                emit_events();
            }
            else
            {
                last_log_event++;
            }
        }
        else
        {
            event_signal.call(LogLevel::Warning, "Could not acquire log mutex after " + std::to_string(last_log_event) + " tries, blocking on lock...");

            std::lock_guard<std::mutex> lock(pending_logs_mutex);
            (void) lock;
            emit_events();
        }
    }

    jw_util::Signal<LogLevel, const std::string &> event_signal;

private:
    unsigned int last_log_event = 0;
    std::mutex pending_logs_mutex;
    std::vector<std::pair<LogLevel, std::string>> pending_logs;

    void emit_events()
    {
        jw_util::Thread::assert_main_thread();

        std::vector<std::pair<LogLevel, std::string>>::const_iterator i = pending_logs.cbegin();
        while (i != pending_logs.cend())
        {
            event_signal.call(i->first, i->second);
            i++;
        }

        pending_logs.clear();
        last_log_event = 0;
    }

    template <typename FirstArgType, typename... ArgTypes>
    static void concat_str(std::string &str, const FirstArgType &first_arg, const ArgTypes & ... args)
    {
        str += to_string(first_arg);
        concat_str(str, args...);
    }

    static void concat_str(std::string &str) {}
};

}

#endif // SONICSOCKET_LOGGER_H
