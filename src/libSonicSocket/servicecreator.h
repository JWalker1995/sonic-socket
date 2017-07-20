#ifndef SERVICECREATOR_H
#define SERVICECREATOR_H

#include <google/protobuf/message_lite.h>

namespace sonic_socket {

class ServiceCreator {
public:
    virtual const google::protobuf::MessageLite &make_summary_message(google::protobuf::Arena &arena);
};

}

#endif // SERVICECREATOR_H
