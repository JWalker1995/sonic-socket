#ifndef MAILBOXINITIALIZER_H
#define MAILBOXINITIALIZER_H

#include "libSonicSocket/messagerouter.h"

namespace sonic_socket {

class MailboxInitializer {
    template <typename... BoxTypes>
    friend class Mailbox;

public:
    MailboxInitializer(Manager &manager)
        : dummy_message_router(manager)
    {}

private:
    MessageRouter dummy_message_router;
};

}

#endif // MAILBOXINITIALIZER_H
