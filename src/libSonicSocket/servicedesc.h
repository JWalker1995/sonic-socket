#ifndef SERVICEDESC_H
#define SERVICEDESC_H

#include <string>

#include <google/protobuf/message_lite.h>

#include "libSonicSocket/servicecreator.h"

namespace sonic_socket {

class BoxLinker;

template <typename SummaryMessageType>
class ServiceDesc : public ServiceCreator {
    static_assert(std::is_base_of<google::protobuf::MessageLite, SummaryMessageType>::value, "SummaryMessageType must be a protobuf Message");

public:
    ServiceDesc(const std::string &interface)
        : interface(interface)
    {}

    const std::string &get_interface() const {
        return interface;
    }

    virtual bool is_enabled();
    virtual const SummaryMessageType &make_summary(google::protobuf::Arena &arena);

    virtual void create(BoxLinker &linker);

private:
    const std::string interface;

    const google::protobuf::MessageLite &make_summary_message(google::protobuf::Arena &arena) {
        return make_summary(arena);
    }
};

}

#endif // SERVICEDESC_H
