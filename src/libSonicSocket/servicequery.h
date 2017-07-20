#ifndef SONICSOCKET_SERVICEQUERY_H
#define SONICSOCKET_SERVICEQUERY_H

#include <string>

#include <google/protobuf/message_lite.h>

namespace sonic_socket {

template <typename SummaryMessageType, typename ScoreType = float>
class ServiceQuery {
    static_assert(std::is_base_of<google::protobuf::MessageLite, SummaryMessageType>::value, "SummaryMessageType must be a protobuf Message");

public:
    ServiceQuery(const std::string &interface)
        : interface(interface)
    {}

    const std::string &get_interface() const {
        return interface;
    }

    virtual ScoreType calc_score(const SummaryMessageType &summmary);

private:
    const std::string interface;
};

}

#endif // SONICSOCKET_SERVICEQUERY_H
