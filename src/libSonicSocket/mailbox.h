#ifndef SONICSOCKET_MAILBOX_H
#define SONICSOCKET_MAILBOX_H

#include <string>

#include "libSonicSocket/messagerouter.h"
#include "libSonicSocket/messageallocator.h"
#include "libSonicSocket/box.h"

#include "libSonicSocket/base.pb.h"

namespace sonic_socket
{

class Manager;

template <typename DerivedType, typename... Types>
class MailboxInternal
{
public:
    MailboxInternal() {}
    
    void init(MailboxInit &mailbox_init) {}

    bool recv_outbox_init(const OutboxInit &outbox_init) {return false;}
    void check_outbox_init() {}
};

template <typename DerivedType, typename FirstType, typename... OtherTypes>
class MailboxInternal<DerivedType, FirstType, OtherTypes...>
    : public FirstType::template Actor<DerivedType>
    , public MailboxInternal<DerivedType, OtherTypes...>
{
public:
    MailboxInternal()
        : FirstType::template Actor<DerivedType>()
        , MailboxInternal<DerivedType, OtherTypes...>()
    {}

    ~MailboxInternal()
    {}

    void init(MailboxInit &mailbox_init)
    {
        FirstType::template Actor<DerivedType>::init(mailbox_init);
        MailboxInternal<DerivedType, OtherTypes...>::init(mailbox_init);
    }

    bool recv_outbox_init(const OutboxInit &outbox_init)
    {
        bool res = FirstType::template Actor<DerivedType>::recv_outbox_init(outbox_init);
        if (res) {return true;}

        return MailboxInternal<DerivedType, OtherTypes...>::recv_outbox_init(outbox_init);
    }

    void check_outbox_init()
    {
        FirstType::template Actor<DerivedType>::check_outbox_init();
        MailboxInternal<DerivedType, OtherTypes...>::check_outbox_init();
    }
};

template <typename... BoxTypes>
class Mailbox : private MailboxInternal<Mailbox<BoxTypes...>, BoxTypes...>
{
private:
    typedef MailboxInternal<Mailbox<BoxTypes...>, BoxTypes...> Internal;

public:
    Mailbox()
    {}

    ~Mailbox()
    {}

    MessageRouter &get_message_router() {return *message_router;}
    Manager &get_manager() {return message_router->get_manager();}

    void set_dummy_message_router(MessageRouter *new_message_router)
    {
        message_router = new_message_router;
    }

    void init(MessageRouter *new_message_router, MailboxInit &mailbox_init)
    {
        message_router = new_message_router;

        Internal::init(mailbox_init);
    }

    void recv_init(const MailboxInit &mailbox_init)
    {
        google::protobuf::RepeatedPtrField<OutboxInit>::const_iterator i = mailbox_init.boxes().cbegin();
        while (i != mailbox_init.boxes().cend())
        {
            bool res = Internal::recv_outbox_init(*i);
            if (!res)
            {
                const std::string msg = create_missing_outbox_error_msg(*i);
                message_router->push_log(LogProxy::LogLevel::Warning, msg);
            }
            i++;
        }

        Internal::check_outbox_init();
    }

    template <typename InboxType>
    MessageRouter::RegisteredInbox generate_inbox_registration()
    {
        MessageRouter::RegisteredInbox registration;
        registration.ptr = static_cast<void *>(this);
        registration.parse_ptr = &parse_func<InboxType>;
        registration.process_ptr = &process_func<InboxType>;
        return registration;
    }

    template <typename BoxType>
    typename BoxType::template Actor<Mailbox<BoxTypes...>> &get_box()
    {
        MailboxInternal<Mailbox<BoxTypes...>, BoxTypes...> *boxes = static_cast<MailboxInternal<Mailbox<BoxTypes...>, BoxTypes...> *>(this);
        typename BoxType::template Actor<Mailbox<BoxTypes...>> *box = static_cast<typename BoxType::template Actor<Mailbox<BoxTypes...>> *>(boxes);
        return *box;
    }

    template <typename BoxType>
    static Mailbox<BoxTypes...> &get_mailbox(typename BoxType::template Actor<Mailbox<BoxTypes...>> *actor)
    {
        return *static_cast<Mailbox<BoxTypes...> *>(actor);
    }

protected:
    MessageRouter *message_router;

private:
    static std::string create_missing_outbox_error_msg(const OutboxInit &outbox_init)
    {
        std::string msg = "No outbox found matching OutboxInit with names=[";

        if (outbox_init.names_size())
        {
            google::protobuf::RepeatedPtrField<std::string>::const_iterator i;
            i = outbox_init.names().cbegin();
            while (i != outbox_init.names().cend())
            {
                msg += *i + ", ";
            }
            msg.pop_back();
            msg.pop_back();
        }

        msg += "], inbox_id=" + std::to_string(outbox_init.inbox_id());
        return msg;
    }

    template <typename MessageType>
    static std::string create_uninitialized_outbox_error_msg()
    {
        std::string msg = "Uninitialized outbox with name=";
        msg += MessageType::descriptor->full_name();
        return msg;
    }

    template <typename InboxType>
    static bool parse_func(void *mailbox_ptr, const char *data, unsigned int size, const google::protobuf::Message *&message, MessageAllocator &allocator, std::string &error_msg)
    {
        Mailbox<BoxTypes...> *mailbox = static_cast<Mailbox<BoxTypes...> *>(mailbox_ptr);

        typename InboxType::MessageType* parse = &allocator.alloc_message<typename InboxType::MessageType>();
        bool res;

        res = parse->ParseFromArray(data, size);
        if (!res)
        {
            error_msg = "Message of type " + parse->GetTypeName() + " is malformed";
            return false;
        }

        res = mailbox->get_box<InboxType>().parse_message(*parse, error_msg);
        if (!res)
        {
            error_msg = "Parsed message of type " + parse->GetTypeName() + " is invalid: " + error_msg;
            return false;
        }

        message = static_cast<const google::protobuf::Message *>(parse);
        return true;
    }

    template <typename InboxType>
    static void process_func(void *mailbox_ptr, const google::protobuf::Message *message)
    {
        Mailbox<BoxTypes...> *mailbox = static_cast<Mailbox<BoxTypes...> *>(mailbox_ptr);
        mailbox->get_box<InboxType>().process_message(*message);
    }
};

}

#endif // SONICSOCKET_MAILBOX_H
