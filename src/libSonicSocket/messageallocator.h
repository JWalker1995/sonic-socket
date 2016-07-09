#ifndef MESSAGEALLOCATOR_H
#define MESSAGEALLOCATOR_H

#include <google/protobuf/arena.h>

namespace sonic_socket
{

class MessageAllocator
{
public:
    template <typename MessageType>
    MessageType &alloc_message()
    {
        static_assert(std::is_base_of<google::protobuf::Message, MessageType>::value, "Message<MessageType>: MessageType must inherit google::protobuf::Message");

        return *google::protobuf::Arena::CreateMessage<MessageType>(&protobuf_arena);
    }

private:
    google::protobuf::Arena protobuf_arena;
};

}

#endif // MESSAGEALLOCATOR_H
