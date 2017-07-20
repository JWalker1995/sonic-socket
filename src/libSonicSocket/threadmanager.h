#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include "libSonicSocket/jw_util/poolfifo.h"
#include "libSonicSocket/jw_util/workqueueinsomniac.h"

#include "libSonicSocket/udpsocket.h"
#include "libSonicSocket/workerrequest.h"

namespace sonic_socket {

class ThreadManager {
    /*
     * A ThreadManager spawns 2 threads:
     *     A polling thread, which runs a blocking UdpSocket::poll() in a loop.
     *         Recieved packets are sent to the worker thread in a WorkerDecodeRequest.
     *     A worker thread, which accepts WorkerRequests and processes them.
    */

public:
    ThreadManager(LogProxy &logger)
        : logger(logger)
    {}

    void connect(const std::string &server) {
        jw_util::Thread::assert_main_thread();

        char *data = new char[server.size() + 1];
        std::copy(server.cbegin(), server.cend(), data);
        data[server.size()] = '\0';

        workers.push(WorkerResolveRequest(data, server.size()));
    }

    void queue_message(MessageRouter &message_router, MessageRouter::InboxId inbox_id, const google::protobuf::Message &message) {
        jw_util::Thread::assert_main_thread();
        //assert(!MessageBuffer::is_inbox_id_holding(inbox_id));

        workers.push(WorkerEncodeRequest(&message_router, inbox_id, &message));
    }

    void send_packet(const Remote &remote, const unsigned char *data, unsigned int size) {
        jw_util::Thread::assert_child_thread();

        logger.push_event<LogProxy::LogLevel::Debug>("Sending packet to ", remote, " of size ", size, " bytes...");

        try
        {
            socket.send(remote, reinterpret_cast<const char *>(data), size);
        }
        catch (const UdpSocket::SendException &ex)
        {
            logger.push_event<LogProxy::LogLevel::Warning>(ex.what());
        }
        catch (const UdpSocket::DropException &ex)
        {
            logger.push_event<LogProxy::LogLevel::Notice>(ex.what());
        }
    }

private:
    UdpSocket socket;

    std::atomic<bool> polling_thread_run;
    std::thread polling_thread;

    jw_util::WorkQueueInsomniac<1, WorkerRequest> workers;

    jw_util::PoolFIFO<WorkerDecodeRequest::PacketData> packet_data_pool;

    LogProxy &logger;
};

}

#endif // THREADMANAGER_H
