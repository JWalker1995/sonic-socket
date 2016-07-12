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
    typedef Outbox<MessageType> SelfType;

    template <typename MailboxType>
    class Actor : public Box::Actor<MailboxType>
    {
    public:
        void after_set_dummy_message_router()
        {
            remote_inbox_id = MailboxType::template get_mailbox<SelfType>(this).get_message_router().register_holding_inbox();
        }

        bool try_mailbox_init(const MailboxInit &mailbox_init)
        {
            google::protobuf::RepeatedPtrField<OutboxInit>::const_iterator i = mailbox_init.boxes().cbegin();
            while (i != mailbox_init.boxes().cend())
            {
                google::protobuf::RepeatedPtrField<std::string>::const_iterator j;
                j = std::find(i->names().cbegin(), i->names().cend(), MessageType::descriptor()->full_name());
                bool match = j != i->names().cend();
                if (match) {return true;}

                i++;
            }

            return false;
        }

        void recv_mailbox_init(const MailboxInit &mailbox_init)
        {
            google::protobuf::RepeatedPtrField<OutboxInit>::const_iterator i = mailbox_init.boxes().cbegin();
            while (i != mailbox_init.boxes().cend())
            {
                google::protobuf::RepeatedPtrField<std::string>::const_iterator j;
                j = std::find(i->names().cbegin(), i->names().cend(), MessageType::descriptor()->full_name());
                bool match = j != i->names().cend();
                if (match)
                {
                    MailboxType::template get_mailbox<SelfType>(this).get_message_router().release_messages(remote_inbox_id, i->inbox_id());
                    remote_inbox_id = i->inbox_id();
                    return;
                }

                i++;
            }

            assert(false);
        }

        void register_and_generate_mailbox_init(MailboxInit &mailbox_init) {(void) mailbox_init;}
        void unregister() {}

        bool is_resolved() const
        {
            return !MessageBuffer::is_inbox_id_holding(remote_inbox_id);
        }


        MessageType &alloc_message()
        {
            return MailboxType::template get_mailbox<SelfType>(this).get_message_router().get_message_allocator().template alloc_message<MessageType>();
        }

        void send_message(const MessageType &message)
        {
            MailboxType::template get_mailbox<SelfType>(this).get_message_router().queue_message(remote_inbox_id, message);
        }

    private:
        MessageRouter::InboxId remote_inbox_id;
    };
};

}

#endif // SONICSOCKET_OUTBOX_H
