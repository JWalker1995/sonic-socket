#ifndef SONICSOCKET_OUTBOX_H
#define SONICSOCKET_OUTBOX_H

#include "libSonicSocket/box.h"
#include "libSonicSocket/mailbox.h"
#include "libSonicSocket/messagebuffer.h"

namespace sonic_socket
{

template <typename _MessageType>
class Outbox : private Box
{
public:
    typedef _MessageType MessageType;

    template <typename MailboxType>
    class Actor : public Box::Actor<MailboxType>
    {
    public:
        void init(MailboxInit &mailbox_init)
        {
            (void) mailbox_init;
            remote_inbox_id = this->get_message_router().alloc_holding_inbox();
        }

        bool recv_outbox_init(const OutboxInit &outbox_init)
        {
            if (is_resolved()) {return false;}

            google::protobuf::RepeatedPtrField<std::string>::const_iterator i;
            i = std::find(outbox_init.names().cbegin(), outbox_init.names().cend(), MessageType::descriptor->full_name());
            bool match = i != outbox_init.names().cend();
            if (match)
            {
                this->get_message_router().release_messages(remote_inbox_id, outbox_init.inbox_id());
                remote_inbox_id = outbox_init.inbox_id();
            }
            return match;
        }

        void check_outbox_init()
        {
            if (!is_resolved())
            {
                const std::string msg = Mailbox<>::create_uninitialized_outbox_error_msg<MessageType>();
                this->get_message_router().push_log(LogProxy::LogLevel::Warning, msg);
            }
        }

        bool is_resolved() const
        {
            return !MessageBuffer::is_inbox_id_holding(remote_inbox_id);
        }


        MessageType &alloc_message()
        {
            return this->get_message_router().get_message_allocator().template alloc_message<MessageType>();
        }

        void send_message(const MessageType &message)
        {
            this->get_message_router().queue_message(remote_inbox_id, message);
        }

    private:
        MessageRouter::InboxId remote_inbox_id;
    };
};

}

#endif // SONICSOCKET_OUTBOX_H