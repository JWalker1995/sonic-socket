#ifndef SONICSOCKET_INBOXASYNCHRONOUS_H
#define SONICSOCKET_INBOXASYNCHRONOUS_H

#include "libSonicSocket/inbox.h"

namespace sonic_socket
{

template <
    typename _MessageType,
    typename ProcessCallbackClass,
    bool (*parse_callback)(const _MessageType &, std::string &),
    void (ProcessCallbackClass::*process_callback)(const _MessageType &)
>
class InboxAsynchronous : public Inbox<_MessageType, parse_callback>
{
    typedef InboxAsynchronous<_MessageType, ProcessCallbackClass, parse_callback, process_callback> SelfType;

public:
    typedef _MessageType MessageType;

    template <typename MailboxType>
    class Actor : public Inbox<_MessageType, parse_callback>::template Actor<MailboxType>
    {
        typedef typename Inbox<_MessageType, parse_callback>::template Actor<MailboxType> BaseType;

    public:
        void register_and_generate_mailbox_init(MailboxInit &mailbox_init)
        {
            BaseType::register_and_generate_mailbox_init(mailbox_init, make_inbox_registration());
        }

        void unregister()
        {
            BaseType::unregister(make_inbox_registration());
        }

        void set_class_pointer(ProcessCallbackClass *class_pointer)
        {
            class_ptr = class_pointer;
        }

        void process_message(const MessageType &message)
        {
            (class_ptr->*process_callback)(message);
        }

    private:
        ProcessCallbackClass *class_ptr;

        MessageRouter::RegisteredInbox make_inbox_registration()
        {
            return MailboxType::template get_mailbox<SelfType>(this).template generate_inbox_registration<SelfType>();
        }
    };
};

}

#endif // SONICSOCKET_INBOXASYNCHRONOUS_H
