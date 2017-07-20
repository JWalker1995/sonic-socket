#ifndef SONICSOCKET_WORKERREQUEST_H
#define SONICSOCKET_WORKERREQUEST_H

#include <string>
#include <string.h>
#include <assert.h>

#include "libSonicSocket/messagerouter.h"
#include "libSonicSocket/fountainbase.h"

namespace sonic_socket
{

class WorkerResolveRequest;
class WorkerDecodeRequest;
class WorkerEncodeRequest;

class WorkerRequest
{
public:
    WorkerRequest(MessageRouter::InboxId inbox_id)
        : inbox_id(inbox_id)
    {}

    WorkerRequest(const WorkerRequest &other)
    {
        memcpy(this, &other, sizeof(WorkerRequest));
    }

    bool is_resolve() const {return inbox_id == coder_request_resolve;}
    bool is_decode() const {return inbox_id == coder_request_decode;}
    bool is_encode() const {return inbox_id != coder_request_resolve && inbox_id != coder_request_decode;}

    const WorkerResolveRequest &to_resolve() const {
        assert(is_resolve());
        return *static_cast<const WorkerResolveRequest*>(this);
    }

    WorkerDecodeRequest &to_decode() {
        // Cannot be const because the packet is eventually de-mangled in FountainSink
        assert(is_decode());
        return *static_cast<WorkerDecodeRequest*>(this);
    }

    const WorkerEncodeRequest &to_encode() const {
        assert(is_encode());
        return *static_cast<const WorkerEncodeRequest*>(this);
    }

protected:
    struct DecodePacketData
    {
        Remote remote;
        FountainBase::Packet packet;
    };

    static constexpr MessageRouter::InboxId coder_request_resolve = static_cast<MessageRouter::InboxId>(-1);
    static constexpr MessageRouter::InboxId coder_request_decode = static_cast<MessageRouter::InboxId>(-2);

    MessageRouter::InboxId inbox_id;

    union
    {
        struct
        {
            const char *data;
            unsigned int size;
        } resolve;

        struct
        {
            DecodePacketData *packet_data;
        } decode;

        struct
        {
            MessageRouter *message_router;
            const google::protobuf::Message *message;
        } encode;
    };
};

class WorkerResolveRequest : public WorkerRequest
{
public:
    WorkerResolveRequest(const char *server_data, unsigned int server_size)
        : WorkerRequest(coder_request_resolve)
    {
        resolve.data = server_data;
        resolve.size = server_size;

        assert(is_resolve());
    }

    const char *get_data() const {return resolve.data;}
    unsigned int get_size() const {return resolve.size;}
};

class WorkerDecodeRequest : public WorkerRequest
{
public:
    typedef DecodePacketData PacketData;

    WorkerDecodeRequest(PacketData *packet_data)
        : WorkerRequest(coder_request_decode)
    {
        decode.packet_data = packet_data;

        assert(is_decode());
    }

    PacketData *get_packet_data() {return decode.packet_data;}
};

class WorkerEncodeRequest : public WorkerRequest
{
public:
    WorkerEncodeRequest(MessageRouter *message_router, MessageRouter::InboxId inbox_id, const google::protobuf::Message *message)
        : WorkerRequest(inbox_id)
    {
        encode.message_router = message_router;
        encode.message = message;

        assert(is_encode());
    }

    MessageRouter *get_message_router() const {return encode.message_router;}
    MessageRouter::InboxId get_inbox_id() const {return inbox_id;}
    const google::protobuf::Message *get_message() const {return encode.message;}
};

}

#endif // SONICSOCKET_WORKERREQUEST_H
