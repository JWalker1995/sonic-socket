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
    static constexpr InboxId error_report_inbox_id = 1;

    MessageRouter(Manager &manager)
        : manager(manager)
        , fountain_coder(manager.get_logger())
    {}

    MessageRouter(Manager &manager, const Remote &remote)
        : manager(manager)
        , remote(remote)
        , fountain_coder(manager.get_logger())
    {
        assert(remote.is_resolved());
    }

    Manager &get_manager() const {
        return manager;
    }

    const Remote &get_remote() const {
        return remote;
    }

    MessageAllocator &get_message_allocator() {
        return manager.get_message_allocator();
    }

    InboxId register_holding_inbox() {
        return manager.get_message_buffer().alloc_holding_inbox_id();
    }

    InboxId register_inbox(const RegisteredInbox &registration) {
        InboxId inbox_id;
        if (free_inboxes.empty())
        {
            inbox_id = inboxes.size();
            inboxes.push_back(registration);
        }
        else
        {
            inbox_id = free_inboxes.back();
            free_inboxes.pop_back();
            inboxes[inbox_id] = registration;
        }
        return inbox_id;
    }

    void unregister_inbox(const RegisteredInbox &registration) {
        std::vector<RegisteredInbox>::const_iterator i = std::find(inboxes.cbegin(), inboxes.cend(), registration);
        assert(i != inboxes.cend());
        unregister_inbox(i - inboxes.cbegin());
    }

    void unregister_inbox(InboxId inbox_id) {
        assert(inbox_id < inboxes.size());
        assert(inboxes[inbox_id].ptr != 0);

        inboxes[inbox_id].ptr = 0;
        free_inboxes.push_back(inbox_id);
    }

    void queue_message(InboxId inbox_id, const google::protobuf::Message &message) {
        jw_util::Thread::assert_main_thread();

        if (MessageBuffer::is_inbox_id_holding(inbox_id))
        {
            manager.get_message_buffer().hold_message(inbox_id, message);
        }
        else
        {
            manager.queue_message(*this, inbox_id, message);
        }
    }

    void release_messages(InboxId holding_id, InboxId resolved_id) {
        manager.get_message_buffer().release_messages(*this, holding_id, resolved_id);
    }

    void send_message(InboxId inbox_id, const google::protobuf::Message &message) {
        jw_util::Thread::assert_child_thread();

        MessageMetaCompressor::Meta meta;
        meta.inbox_id = inbox_id;
        meta.size = message.ByteSize();

        char *data = message_encoder.alloc_message(meta);
        message.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8 *>(data));
        message_encoder.send_message(fountain_coder);

        send_packet();
    }

    void send_packet() {
        FountainCoder::Packet packet;
        fountain_coder.generate_packet(packet);

        manager.send_packet(remote, packet.get_data(), packet.get_size());

        last_send = std::chrono::steady_clock::now();
    }

    void receive_packet(FountainCoder::Packet &packet) {
        struct ParsedMessage
        {
            const RegisteredInbox *inbox;
            const google::protobuf::Message *protobuf;
        };

        static thread_local std::vector<ParsedMessage> parsed_messages;
        static thread_local std::string error_str;
        parsed_messages.clear();

        FountainCoder::DecodedPacket decode;
        bool succ = fountain_coder.recv_packet(packet, decode);
        if (!succ) {return;}

        while (message_decoder.extract_message(decode))
        {
            assert(message_decoder.has_message());

            InboxId inbox_id = message_decoder.get_message_meta().inbox_id;
            static_assert(std::numeric_limits<InboxId>::min() == 0, "InboxId must be unsigned");
            if (inbox_id >= inboxes.size())
            {
                std::string error_msg = "Invalid inbox_id (" + std::to_string(inbox_id) + " too high)";
                fountain_coder.report_decode_error(decode, error_msg);
                return;
            }

            ParsedMessage message;
            message.inbox = &inboxes[inbox_id];

            if (message.inbox->ptr == 0)
            {
                std::string error_msg = "Invalid inbox_id (" + std::to_string(inbox_id) + " was unregistered)";
                fountain_coder.report_decode_error(decode, error_msg);
                return;
            }

            const char *data = message_decoder.get_message_data();
            unsigned int size = message_decoder.get_message_meta().size;
            bool parsed = message.inbox->parse_ptr(message.inbox->ptr, data, size, message.protobuf, manager.get_message_allocator(), error_str);

            if (!parsed)
            {
                std::string error_msg = "Could not parse message: " + error_str;
                fountain_coder.report_decode_error(decode, error_msg);
                return;
            }

            parsed_messages.push_back(message);

            message_decoder.clear_message();
        }

        if (message_decoder.has_error())
        {
            std::string error_msg = "Could not decode symbols into message: " + message_decoder.get_error_string();
            fountain_coder.report_decode_error(decode, error_msg);
            return;
        }

        std::vector<ParsedMessage>::const_iterator i = parsed_messages.cbegin();
        while (i != parsed_messages.cend())
        {
            i->inbox->process_ptr(i->inbox->ptr, i->protobuf);
            i++;
        }

        last_recv = std::chrono::steady_clock::now();
    }

    template <LogProxy::LogLevel level>
    void push_log_event(const std::string &str) {
        manager.get_logger().push_event<level>(str);
    }

    bool has_data_to_send() const {
        return fountain_coder.has_data_to_send();
    }

    bool is_remote_timed_out(std::chrono::steady_clock::time_point threshold) const {
        return last_recv < threshold;
    }

    bool is_self_timed_out(std::chrono::steady_clock::time_point threshold) const {
        return last_send < threshold;
    }

    bool is_dummy() const {
        return !remote.is_resolved();
    }

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
