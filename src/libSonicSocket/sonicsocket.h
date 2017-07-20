#ifndef SONICSOCKET_H
#define SONICSOCKET_H

#include <string>

#include "libSonicSocket/workermanager.h"

namespace sonic_socket {

class SonicSocket
{
public:
    SonicSocket(Remote::Port port = SS_DEFAULT_PORT) {
    }

    void connect(const std::string &server) {
        worker_manager.connect(server);
    }

private:
    WorkerManager worker_manager;
};

}

#endif // SONICSOCKET_H
