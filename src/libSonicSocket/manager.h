#ifndef SONICSOCKET_MANAGER_H
#define SONICSOCKET_MANAGER_H

#include <string>
#include <vector>
#include <queue>
#include <array>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>

#include <google/protobuf/message.h>

#include "libSonicSocket/jw_util/methodcallback.h"
#include "libSonicSocket/jw_util/pool.h"
#include "libSonicSocket/jw_util/poolfifo.h"
#include "libSonicSocket/jw_util/signal.h"
#include "libSonicSocket/jw_util/stackbasedvector.h"
#include "libSonicSocket/jw_util/workqueueinsomniac.h"

#include "libSonicSocket/config/SS_DEFAULT_PORT.h"

#include "libSonicSocket/udpsocket.h"
#include "libSonicSocket/remote.h"
#include "libSonicSocket/serverconnection.h"
#include "libSonicSocket/messageallocator.h"
#include "libSonicSocket/messagebuffer.h"
#include "libSonicSocket/modulelookup.h"
#include "libSonicSocket/module.h"
#include "libSonicSocket/logproxy.h"
#include "libSonicSocket/workerrequest.h"

namespace sonic_socket
{

class Manager
{
public:
    enum class Type {Client, Server};

    Manager(Type type, Remote::Port port = SS_DEFAULT_PORT);
    ~Manager();

    void add_server(const std::string &server);

    // ModuleLookup &alloc_module_lookup();

    // TODO: Move to another class
    /*
    template <typename... BoxTypes>
    float lookup_module(Mailbox<BoxTypes...> &mailbox, ModuleLookup &lookup)
    {
        jw_util::Thread::assert_main_thread();

        float score;
        RegisteredModule *module = lookup_module_local(lookup.get_request(), score);

        if (!module)
        {
            logger.push_event(LogProxy::LogLevel::Error, "Could not find any registered modules matching lookup:\n" + lookup.to_string());
            return 0.0f;
        }

        ModuleConnectionRequest &request = alloc_message<ModuleConnectionRequest>();
        request.set_module_id(module->module_id);
        mailbox.register_inbox_ids(request.inbox_ids());
        request.mutable_coords()->CopyFrom(lookup.get_request().coords());

        module->server_connection.queue_message(ServerConnection::module_connection_inbox_id, request);
        module->server_connection.send_messages();

        module_lookup_pool.free(lookup);

        return score;
    }
    */

    /*
    void load_module(const std::string &interface, unsigned int version, const ModuleSpaceCoordinate *coords, unsigned int num_coords, jw_util::MethodCallback<ModuleInit> instantiate_callback);
    void list_loaded_modules(ServerConnection &server_connection);
    void init_module_connection(ServerConnection &server_connection, const ModuleConnectionRequest &message);

    void register_module(ServerConnection &server_connection, const ModuleRegistration &message);
    */

    template <typename... BoxTypes>
    void init_mailbox(Mailbox<BoxTypes...> &mailbox)
    {
        mailbox.set_dummy_message_router(&dummy_message_router);
    }

    void queue_message(MessageRouter &message_router, MessageRouter::InboxId inbox_id, const google::protobuf::Message &message);

    void send_packet(const Remote &remote, const unsigned char *data, unsigned int size);

    void tick();

    LogProxy &get_logger() {return logger;}

    MessageAllocator &get_message_allocator() {return message_allocator;}
    MessageBuffer &get_message_buffer() {return message_buffer;}

    jw_util::Signal<ServerConnection &> new_connection_signal;

private:
    Type type;

    UdpSocket socket;

    std::unordered_map<Remote, ServerConnection, Remote::Hasher> servers_map;

    //jw_util::Pool<ModuleLookup> module_lookup_pool;

    std::atomic<bool> polling_thread_run;
    std::thread polling_thread;

    jw_util::WorkQueueInsomniac<1, WorkerRequest> workers;

    jw_util::PoolFIFO<WorkerDecodeRequest::PacketData> packet_data_pool;

    MessageAllocator message_allocator;
    MessageBuffer message_buffer;

    LogProxy logger;

    MessageRouter dummy_message_router;

    static Manager *instance;

    /*
    struct RegisteredModule
    {
        RegisteredModule(ServerConnection &server_connection, ModuleId module_id, const google::protobuf::RepeatedPtrField<ModuleSpaceCoordinate> &coords)
            : server_connection(server_connection)
            , module_id(module_id)
            , coords(coords.cbegin(), coords.cend())
        {}

        ServerConnection &server_connection;
        ModuleId module_id;
        std::vector<ModuleSpaceCoordinate> coords;
    };
    std::mutex register_module_mutex;
    std::unordered_multimap<std::string, RegisteredModule> registered_modules;

    struct LoadedModule
    {
        ModuleRegistration message;
        jw_util::MethodCallback<ModuleInit> instantiate_callback;
    };

    std::mutex loaded_modules_mutex;
    std::vector<LoadedModule> loaded_modules;
    */

    ServerConnection *find_server_connection(const Remote &remote, bool create);

    void thread_poll_socket();

    void resolve_server(const char *server_data, unsigned int server_size);
    void worker_send_message(ServerConnection *server_connection, MessageRouter::InboxId inbox_id, const google::protobuf::Message *message);
    void worker_func(WorkerRequest request);
    void interval_func();

    /*
    //bool lookup_module(ModuleLookupRequest &request, const ModuleRegisterRequest *result_module, float &result_score) const;
    RegisteredModule *lookup_module_local(ModuleLookupRequest &request, float &score);

    template <typename ContainerType>
    void check_module_coords(const ContainerType &container)
    {
#ifndef NDEBUG
        typename ContainerType::const_iterator i = container.cbegin();
        while (i != container.cend())
        {
            bool missing_key = i->key().empty();
            bool missing_weight = -0.000001 < i->weight() && i->weight() < 0.000001;
            bool missing_string_value = i->string_value().empty();
            bool missing_scalar_value = -0.000001 < i->scalar_value() && i->scalar_value() < 0.000001;

            if (missing_key)
            {
                logger.push_event(LogProxy::LogLevel::Warning, "ModuleSpaceCoordinate::key is empty");
            }

            if (missing_weight)
            {
                // LogLevel::Info because could happen if weight == 0
                logger.push_event(LogProxy::LogLevel::Info, "ModuleSpaceCoordinate::weight is zero");
            }

            if (missing_string_value && missing_scalar_value)
            {
                // LogLevel::Info because could happen if scalar_value == 0
                logger.push_event(LogProxy::LogLevel::Info, "ModuleSpaceCoordinate::string_value is empty and ModuleSpaceCoordinate::scalar_value is zero");
            }

            if (!missing_string_value && !missing_scalar_value)
            {
                logger.push_event(LogProxy::LogLevel::Warning, "ModuleSpaceCoordinate::string_value and ModuleSpaceCoordinate::scalar_value are both set");
            }

            i++;
        }
#endif
    }

    template <typename ContainerType>
    static void sort_by_key(ContainerType &container)
    {
        std::sort(container.begin(), container.end(), &compare_by_key<const typename ContainerType::value_type&>);
    }
    template <typename IteratorType>
    static bool compare_by_key(IteratorType a, IteratorType b)
    {
        return a.key() < b.key();
    }
    */

    /*
    static bool lookup_module_matches(const google::protobuf::RepeatedPtrField<ModuleLookupRequest_MatchEntry> &req_match, const google::protobuf::RepeatedPtrField<ModuleRegisterRequest_MatchEntry> &desc_match);
    static float lookup_module_score(const google::protobuf::RepeatedPtrField<ModuleLookupRequest_MinimizeErrorsSqEntry> &req_minimize_error_sq, const google::protobuf::RepeatedPtrField<ModuleRegisterRequest_MinimizeErrorsSqEntry> &desc_minimize_error_sq);
    */

    static void log_handler(google::protobuf::LogLevel protobuf_level, const char *filename, int line, const std::string &protobuf_message);
};

}

#endif // SONICSOCKET_MANAGER_H
