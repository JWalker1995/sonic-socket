#ifndef SONICSOCKET_BOX_H
#define SONICSOCKET_BOX_H

#include "libSonicSocket/messagerouter.h"

namespace sonic_socket
{

class Manager;

class Box
{
public:
	Box() = delete;

    template <typename MailboxType>
    class Actor
    {
    protected:
    	MailboxType &get_mailbox() {return *static_cast<MailboxType *>(this);}
        MessageRouter &get_message_router() {return *get_mailbox().message_router;}
    };
};

}

#endif // SONICSOCKET_BOX_H
