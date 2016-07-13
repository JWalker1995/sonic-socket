#include "messagebuffer.h"

namespace sonic_socket
{

MessageRouter::InboxId MessageBuffer::alloc_holding_inbox_id()
{
    assert(is_inbox_id_holding(next_holding_inbox_id));
    return next_holding_inbox_id--;
}

void MessageBuffer::hold_message(MessageRouter::InboxId inbox_id, const google::protobuf::Message &message)
{
    assert(is_inbox_id_holding(inbox_id));

    held_messages.emplace_back(inbox_id, &message);
}

void MessageBuffer::release_messages(MessageRouter &message_router, MessageRouter::InboxId holding_id, MessageRouter::InboxId resolved_id)
{
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

}
