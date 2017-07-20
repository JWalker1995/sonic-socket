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

    void init_handlers() {
        mailbox.get_box<MailboxInitInbox>().set_class_pointer(this);
        InboxId inbox_id_1 = register_inbox(mailbox.generate_inbox_registration<MailboxInitInbox>());
        assert(inbox_id_1 == mailbox_init_inbox_id);

        mailbox.get_box<ErrorReportInbox>().set_class_pointer(this);
        InboxId inbox_id_2 = register_inbox(mailbox.generate_inbox_registration<ErrorReportInbox>());
        assert(inbox_id_2 == error_report_inbox_id);

        /*
        {
            RegisteredInbox registration;
            registration.ptr = 0;

            auto callback = jw_util::MethodCallback<const ModuleConnectionRequest &>::create<ServerConnection, &init_module>(this);
            InboxId inbox_id = register_inbox(callback);
            assert(inbox_id == module_connection_inbox_id);
        }

        {
            auto callback = jw_util::MethodCallback<const ConnectionInit &>::create<ServerConnection, &init_connection>(this);
            InboxId inbox_id = register_inbox(callback);
            assert(inbox_id == connection_init_inbox_id);
        }

        {
            auto callback = jw_util::MethodCallback<const ModuleRegistration &>::create<ServerConnection, &register_module>(this);
            InboxId inbox_id = register_inbox(callback);
            assert(inbox_id == module_registration_inbox_id);
        }

        // Send initialization message
        ConnectionInit message;
        send_message(connection_init_inbox_id, message);
        */
    }

    /*
    void init_module(const ModuleConnectionRequest &message)
    {
        manager.init_module_connection(*this, message);
    }

    void init_connection(const ConnectionInit &message)
    {
        (void) message;
        manager.list_loaded_modules(*this);
    }

    void register_module(const ModuleRegistration &message)
    {
        manager.register_module(*this, message);
    }
    */



    template <typename... BoxTypes>
    void register_mailbox(Mailbox<BoxTypes...> &mailbox)
    {
        jw_util::Thread::assert_child_thread();

        pending_mailboxes.push_back(mailbox.make_mailbox_init_receiver());

        MailboxInit &mailbox_init = get_message_allocator().alloc_message<MailboxInit>();
        mailbox_init.set_signature(signature);
        mailbox.set_final_message_router(this, mailbox_init);
        send_message(MessageRouter::mailbox_init_inbox_id, mailbox_init);
    }

private:
    static const std::string signature = "spatula pickle peach bear";

    static bool parse_mailbox_init(const MailboxInit &message, std::string &error_str)
    {
        if (message.signature() != signature) {
            error_str = "Incorrect message signature";
            return false;
        }

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

    static bool parse_error_report(const ErrorReport &message, std::string &error_str) {
        if (message.level() == ErrorReport_Level_DEBUG) {
            error_str = "Error report level is unknown";
            return false;
        }

        if (message.text().empty()) {
            error_str = "Error report text is empty";
            return false;
        }

        return true;
    }

    void process_error_report(const ErrorReport &message) {
        switch (message.level()) {
        case ErrorReport_Level_UNKNOWN:
            assert(false);
            break;
        case ErrorReport_Level_DEBUG:
            push_log_event<LogProxy::LogLevel::Debug>(message.text());
            break;
        case ErrorReport_Level_NOTICE:
            push_log_event<LogProxy::LogLevel::Notice>(message.text());
            break;
        case ErrorReport_Level_WARNING:
            push_log_event<LogProxy::LogLevel::Warning>(message.text());
            break;
        case ErrorReport_Level_ERROR:
            push_log_event<LogProxy::LogLevel::Error>(message.text());
            break;
        case ErrorReport_Level_FATAL:
            push_log_event<LogProxy::LogLevel::Fatal>(message.text());
            break;
        default:
            assert(false);
        }
    }

    typedef InboxAsynchronous<MailboxInit, ServerConnection, &parse_mailbox_init, &process_mailbox_init> MailboxInitInbox;
    typedef InboxAsynchronous<ErrorReport, ServerConnection, &parse_error_report, &process_error_report> ErrorReportInbox;
    Mailbox<MailboxInitInbox, ErrorReportInbox> mailbox;

    std::vector<jw_util::MethodCallback<const MailboxInit &, bool &>> pending_mailboxes;

    /*
    void callback_init_module(const ModuleConnectionRequest &message);
    void callback_init_connection(const ConnectionInit &message);
    void callback_register_module(const ModuleRegistration &message);
    */
};

}

#endif // SONICSOCKET_SERVERCONNECTION_H
