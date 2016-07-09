#include "serverconnection.h"

#include "libSonicSocket/jw_util/fastmath.h"

#include "libSonicSocket/manager.h"
#include "libSonicSocket/base.pb.h"

namespace sonic_socket
{

void ServerConnection::init_handlers()
{
    RegisteredInbox registration;
    InboxId inbox_id;

    registration.ptr = static_cast<void *>(this);
    registration.parse_ptr = 0;
    registration.process_ptr = 0;
    inbox_id = register_inbox(registration);
    assert(inbox_id == module_registration_inbox_id);

    /*
    {
        RegisteredInbox registration;
        registration.ptr = 0;
        
        auto callback = jw_util::MethodCallback<const ModuleConnectionRequest &>::create<ServerConnection, &ServerConnection::init_module>(this);
        InboxId inbox_id = register_inbox(callback);
        assert(inbox_id == module_connection_inbox_id);
    }

    {
        auto callback = jw_util::MethodCallback<const ConnectionInit &>::create<ServerConnection, &ServerConnection::init_connection>(this);
        InboxId inbox_id = register_inbox(callback);
        assert(inbox_id == connection_init_inbox_id);
    }

    {
        auto callback = jw_util::MethodCallback<const ModuleRegistration &>::create<ServerConnection, &ServerConnection::register_module>(this);
        InboxId inbox_id = register_inbox(callback);
        assert(inbox_id == module_registration_inbox_id);
    }

    // Send initialization message
    ConnectionInit message;
    send_message(connection_init_inbox_id, message);
    */
}

/*
void ServerConnection::init_module(const ModuleConnectionRequest &message)
{
    manager.init_module_connection(*this, message);
}

void ServerConnection::init_connection(const ConnectionInit &message)
{
    (void) message;
    manager.list_loaded_modules(*this);
}

void ServerConnection::register_module(const ModuleRegistration &message)
{
    manager.register_module(*this, message);
}
*/

}
