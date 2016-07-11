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
public:
    typedef _MessageType MessageType;
    typedef InboxAsynchronous<_MessageType, ProcessCallbackClass, parse_callback, process_callback> SelfType;

    template <typename MailboxType>
    class Actor : public Inbox<_MessageType, parse_callback>::template Actor<MailboxType>
    {
    public:
        void init(MailboxInit &mailbox_init)
        {
            init(mailbox_init, make_inbox_registration());
        }

        void deinit()
        {
            deinit(make_inbox_registration());
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
