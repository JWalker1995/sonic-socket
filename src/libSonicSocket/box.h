#ifndef SONICSOCKET_BOX_H
#define SONICSOCKET_BOX_H

#include "libSonicSocket/base.pb.h"

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
    };
};

}

#endif // SONICSOCKET_BOX_H
