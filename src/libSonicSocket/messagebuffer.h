#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H

#include <deque>

#include "libSonicSocket/messagerouter.h"

namespace sonic_socket
{

class MessageBuffer
{
public:
    MessageRouter::InboxId alloc_holding_inbox_id() {
        assert(is_inbox_id_holding(next_holding_inbox_id));
        return next_holding_inbox_id--;
    }

    void hold_message(MessageRouter::InboxId inbox_id, const google::protobuf::Message &message) {
        assert(is_inbox_id_holding(inbox_id));

        held_messages.emplace_back(inbox_id, &message);
    }

    void release_messages(MessageRouter &message_router, MessageRouter::InboxId holding_id, MessageRouter::InboxId resolved_id) {
        jw_util::Thread::assert_child_thread();
        assert(is_inbox_id_holding(holding_id));
        assert(!is_inbox_id_holding(resolved_id));

        std::deque<HeldMessage>::const_iterator i = held_messages.cbegin();
        while (i != held_messages.cend())
        {
            if (i->holding_inbox_id == holding_id)
            {
                message_router.send_message(resolved_id, *i->message);
                i = held_messages.erase(i);
            }
            else
            {
                i++;
            }
        }
    }

    static bool is_inbox_id_holding(MessageRouter::InboxId inbox_id) {
        return inbox_id >> (sizeof(MessageRouter::InboxId) * CHAR_BIT - 1);
    }

private:
    struct HeldMessage {
        HeldMessage(MessageRouter::InboxId holding_inbox_id, const google::protobuf::Message *message)
            : holding_inbox_id(holding_inbox_id)
            , message(message)
        {}

        MessageRouter::InboxId holding_inbox_id;
        const google::protobuf::Message *message;
    };

    std::deque<HeldMessage> held_messages;
    MessageRouter::InboxId next_holding_inbox_id = static_cast<MessageRouter::InboxId>(-1);
};

}

#endif // MESSAGEBUFFER_H
