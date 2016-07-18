#ifndef SONICSOCKET_MESSAGEROUTER_H
#define SONICSOCKET_MESSAGEROUTER_H

#include <chrono>

#include <google/protobuf/message.h>

#include "libSonicSocket/remote.h"
#include "libSonicSocket/fountaincoder.h"
#include "libSonicSocket/messageallocator.h"
#include "libSonicSocket/messagedecoder.h"
#include "libSonicSocket/messageencoder.h"

namespace sonic_socket
{

class Manager;
class MessageAllocator;

class MessageRouter
{
public:
    typedef unsigned int MailboxId;
    typedef unsigned int InboxId;

    struct RegisteredInbox
    {
        void *ptr;
        bool (*parse_ptr)(void *, const char *, unsigned int, const google::protobuf::Message *&, MessageAllocator &, std::string &);
        void (*process_ptr)(void *, const google::protobuf::Message *);

        bool operator==(const RegisteredInbox &other) const
        {
            return ptr == other.ptr && parse_ptr == other.parse_ptr && process_ptr == other.process_ptr;
        }
    };

    static constexpr InboxId mailbox_init_inbox_id = 0;
    static constexpr InboxId module_registration_inbox_id = 1;
    static constexpr InboxId module_registration_complete_inbox_id = 2;

    MessageRouter(Manager &manager);
    MessageRouter(Manager &manager, const Remote &remote);

    Manager &get_manager() const {return manager;}
    const Remote &get_remote() const {return remote;}

    MessageAllocator &get_message_allocator();

    InboxId register_holding_inbox();
    InboxId register_inbox(const RegisteredInbox &registration);
    void unregister_inbox(const RegisteredInbox &registration);
    void unregister_inbox(InboxId inbox_id);

    void queue_message(InboxId inbox_id, const google::protobuf::Message &message);
    void release_messages(InboxId holding_id, InboxId resolved_id);

    void send_message(InboxId inbox_id, const google::protobuf::Message &message);

    void send_packet();
    void receive_packet(FountainCoder::Packet &packet);

    template <LogProxy::LogLevel level>
    void push_log_event(const std::string &str);

    bool has_data_to_send() const {return fountain_coder.has_data_to_send();}

    bool is_remote_timed_out(std::chrono::steady_clock::time_point threshold) const
    {
        return last_recv < threshold;
    }
    bool is_self_timed_out(std::chrono::steady_clock::time_point threshold) const
    {
        return last_send < threshold;
    }

    bool is_dummy() const {return !remote.is_resolved();}

private:
    Manager &manager;

    const Remote remote;

    FountainCoder fountain_coder;
    MessageDecoder message_decoder;
    MessageEncoder message_encoder;

    std::vector<RegisteredInbox> inboxes;
    std::vector<InboxId> free_inboxes;

    std::chrono::steady_clock::time_point last_recv = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point last_send = std::chrono::steady_clock::now();
};

}

#endif // SONICSOCKET_MESSAGEROUTER_H
