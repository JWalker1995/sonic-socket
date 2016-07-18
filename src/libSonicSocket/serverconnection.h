#ifndef SONICSOCKET_SERVERCONNECTION_H
#define SONICSOCKET_SERVERCONNECTION_H

#include <vector>

#include "libSonicSocket/jw_util/methodcallback.h"
#include "libSonicSocket/jw_util/thread.h"

#include "libSonicSocket/messagerouter.h"
#include "libSonicSocket/mailbox.h"
#include "libSonicSocket/inboxasynchronous.h"
#include "libSonicSocket/outbox.h"

#include "libSonicSocket/base.pb.h"

namespace sonic_socket
{

class ServerConnection : public MessageRouter
{
public:
    ServerConnection(Manager &manager, const Remote &remote)
        : MessageRouter(manager, remote)
    {}

    void init_handlers();

    template <typename... BoxTypes>
    void register_mailbox(Mailbox<BoxTypes...> &mailbox)
    {
        jw_util::Thread::assert_child_thread();

        pending_mailboxes.push_back(mailbox.make_mailbox_init_receiver());
        mailbox.set_final_message_router(this);
    }

private:
    static bool parse_mailbox_init(const MailboxInit &message, std::string &error_str)
    {
        (void) message;
        (void) error_str;
        return true;
    }

    void process_mailbox_init(const MailboxInit &message)
    {
        std::vector<jw_util::MethodCallback<const MailboxInit &, bool &>>::const_iterator i = pending_mailboxes.cbegin();
        while (i != pending_mailboxes.cend())
        {
            bool finished;
            i->call(message, finished);
            if (finished)
            {
                pending_mailboxes.erase(i);
                return;
            }
            i++;
        }

        push_log_event<LogProxy::LogLevel::Warning>("Received MailboxInit that didn't match any un-initialized mailboxes");
    }

    typedef InboxAsynchronous<MailboxInit, ServerConnection, &parse_mailbox_init, &ServerConnection::process_mailbox_init> MailboxInitInbox;
    Mailbox<MailboxInitInbox> mailbox;

    std::vector<jw_util::MethodCallback<const MailboxInit &, bool &>> pending_mailboxes;

    /*
    void callback_init_module(const ModuleConnectionRequest &message);
    void callback_init_connection(const ConnectionInit &message);
    void callback_register_module(const ModuleRegistration &message);
    */
};

}

#endif // SONICSOCKET_SERVERCONNECTION_H
