#ifndef SONICSOCKET_INBOX_H
#define SONICSOCKET_INBOX_H

#include <string>

#include "libSonicSocket/jw_util/methodcallback.h"

#include "libSonicSocket/box.h"
#include "libSonicSocket/messagerouter.h"
#include "libSonicSocket/module.h"

#include "libSonicSocket/base.pb.h"

namespace sonic_socket
{

template <
    typename _MessageType,
    bool (*parse_callback)(const _MessageType &, std::string &)
    >
class Inbox : private Box
{
public:
    typedef _MessageType MessageType;
    typedef Inbox<MessageType, parse_callback> SelfType;

    template <typename MailboxType>
    class Actor : public Box::Actor<MailboxType>
    {
    public:
        void after_set_dummy_message_router() {}
        bool try_mailbox_init(const MailboxInit &mailbox_init) {(void) mailbox_init; return true;}
        void recv_mailbox_init(const MailboxInit &mailbox_init) {(void) mailbox_init;}

        void register_and_generate_mailbox_init(MailboxInit &mailbox_init, const MessageRouter::RegisteredInbox &registration)
        {
            OutboxInit *outbox_init = mailbox_init.add_boxes();

            // Register inbox with message router
            MessageRouter::InboxId inbox_id = MailboxType::template get_mailbox<SelfType>(this).get_message_router().register_inbox(registration);
            outbox_init->set_inbox_id(inbox_id);

            // Add message name (like sonic_socket.InitialPacket)
            const google::protobuf::Descriptor *descriptor = MessageType::descriptor();
            outbox_init->add_names(descriptor->full_name());
            
            // Add message aliases
            const google::protobuf::MessageOptions &opts = descriptor->options();
            unsigned int num_aliases = opts.ExtensionSize(alias);
            for (unsigned int i = 0; i < num_aliases; i++)
            {
                std::string alias_name = opts.GetExtension(alias, i);
                outbox_init->add_names(alias_name);
            }
        }

        void unregister(const MessageRouter::RegisteredInbox &registration)
        {
            MailboxType::template get_mailbox<SelfType>(this).get_message_router().unregister_inbox(registration);
        }

        static bool parse_message(const MessageType &message, std::string &error_msg)
        {
            return parse_callback(message, error_msg);
        }
    };
};

template <typename MessageType>
bool inbox_noop_parser(const MessageType &message, std::string &error_msg)
{
    (void) message;
    (void) error_msg;
    return true;
}

}

#endif // SONICSOCKET_INBOX_H
