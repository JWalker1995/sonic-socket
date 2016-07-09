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

class LogProxy
{
public:
    enum class LogLevel {Info, Warning, Error, Fatal};

    void push_log(LogLevel level, const std::string &str)
    {
        std::lock_guard<std::mutex> lock(pending_logs_mutex);
        (void) lock;
        pending_logs.emplace_back(level, str);
    }

    void tick()
    {
        if (last_log_event < SS_LOGPROXY_FORCE_LOG_INTERVAL)
        {
            std::unique_lock<std::mutex> lock(pending_logs_mutex, std::try_to_lock);
            if (lock.owns_lock())
            {
                emit_logs();
            }
            else
            {
                last_log_event++;
            }
        }
        else
        {
            log_event.call(LogLevel::Warning, "Could not acquire log mutex after " + std::to_string(last_log_event) + " tries, blocking on lock...");

            std::lock_guard<std::mutex> lock(pending_logs_mutex);
            (void) lock;
            emit_logs();
        }
    }

    jw_util::Signal<LogLevel, const std::string &> log_event;

private:
    unsigned int last_log_event = 0;
    std::mutex pending_logs_mutex;
    std::vector<std::pair<LogLevel, std::string>> pending_logs;

    void emit_logs()
    {
        jw_util::Thread::assert_main_thread();

        std::vector<std::pair<LogLevel, std::string>>::const_iterator i = pending_logs.cbegin();
        while (i != pending_logs.cend())
        {
            log_event.call(i->first, i->second);
            i++;
        }

        pending_logs.clear();
        last_log_event = 0;
    }
};

}

#endif // SONICSOCKET_LOGGER_H
