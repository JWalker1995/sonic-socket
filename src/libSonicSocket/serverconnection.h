#ifndef SONICSOCKET_SERVERCONNECTION_H
#define SONICSOCKET_SERVERCONNECTION_H

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
    static constexpr InboxId module_registration_inbox_id = 0;
    static constexpr InboxId module_registration_complete_inbox_id = 1;

    ServerConnection(Manager &manager, const Remote &remote)
        : MessageRouter(manager, remote)
    {}

    void init_handlers();

private:
    static bool parse_connection(const ModuleConnectionRequest &message, std::string &error_str)
    {
        (void) message;
        (void) error_str;
        return true;
    }

    void process_connection(const ModuleConnectionRequest &message)
    {
        (void) message;
    }

    typedef InboxAsynchronous<ModuleConnectionRequest, ServerConnection, &parse_connection, &ServerConnection::process_connection> ConnectionInbox;
    Mailbox<ConnectionInbox> mailbox;

    /*
    void callback_init_module(const ModuleConnectionRequest &message);
    void callback_init_connection(const ConnectionInit &message);
    void callback_register_module(const ModuleRegistration &message);
    */
};

}

#endif // SONICSOCKET_SERVERCONNECTION_H
