#include "manager.h"

#include "libSonicSocket/config/SS_MAX_PACKET_SIZE.h"
#include "libSonicSocket/config/SS_DEFAULT_PORT.h"

#include "libSonicSocket/serverconnection.h"
#include "libSonicSocket/base.pb.h"

namespace sonic_socket
{

Manager *Manager::instance = 0;

Manager::Manager(Type type, Remote::Port port)
    : type(type)
    , workers(jw_util::MethodCallback<WorkerRequest>::create<Manager, &Manager::worker_func>(this), jw_util::WorkQueueInsomniac<1, WorkerRequest>::construct_paused)
    , dummy_message_router(*this)
{
    assert(!instance);
    instance = this;

    assert(dummy_message_router.is_dummy());

    jw_util::Thread::set_main_thread();

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    google::protobuf::SetLogHandler(&log_handler);

    FountainBase::static_init(logger);

    try
    {
        socket.open(port);
    }
    catch (const std::exception &ex)
    {
        logger.push_event<LogProxy::LogLevel::Fatal>(ex.what());
        return;
    }

    polling_thread_run = true;
    polling_thread = std::thread(&sonic_socket::Manager::thread_poll_socket, this);

    workers.set_wakeup_worker(jw_util::MethodCallback<>::create<Manager, &Manager::interval_func>(this));
    workers.set_wakeup_interval(std::chrono::milliseconds(250));

    workers.start();
}

Manager::~Manager()
{
    assert(instance);
    instance = 0;

    jw_util::Thread::assert_main_thread();

    socket.close();
    polling_thread_run = false;
    polling_thread.join();
    workers.pause();

    google::protobuf::ShutdownProtobufLibrary();
}

void Manager::add_server(const std::string &server)
{
    jw_util::Thread::assert_main_thread();

    char *data = new char[server.size() + 1];
    std::copy(server.cbegin(), server.cend(), data);
    data[server.size()] = '\0';

    workers.push(WorkerResolveRequest(data, server.size()));
}

/*
ModuleLookup &Manager::alloc_module_lookup()
{
    jw_util::Thread::assert_main_thread();

    return module_lookup_pool.alloc();
}
*/

void Manager::queue_message(MessageRouter &message_router, MessageRouter::InboxId inbox_id, const google::protobuf::Message &message)
{
    jw_util::Thread::assert_main_thread();
    assert(!MessageBuffer::is_inbox_id_holding(inbox_id));

    workers.push(WorkerEncodeRequest(&message_router, inbox_id, &message));
}

void Manager::send_packet(const Remote &remote, const unsigned char *data, unsigned int size)
{
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

ServerConnection *Manager::find_server_connection(const Remote &remote, bool create)
{
    jw_util::Thread::assert_child_thread();

    std::unordered_map<Remote, ServerConnection, Remote::Hasher>::iterator found = servers_map.find(remote);
    if (found == servers_map.end())
    {
        if (create)
        {
            auto insert = servers_map.emplace(remote, ServerConnection(*this, remote));
            assert(insert.second);

            ServerConnection &server_connection = insert.first->second;
            assert(!server_connection.is_dummy());
            server_connection.init_handlers();
            new_connection_signal.call(server_connection);
            return &server_connection;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return &found->second;
    }
}

void Manager::tick()
{
    jw_util::Thread::assert_main_thread();

    logger.tick();
}


void Manager::thread_poll_socket()
{
    jw_util::Thread::assert_child_thread();

    WorkerDecodeRequest::PacketData *packet_data = &packet_data_pool.alloc();

    while (polling_thread_run)
    {
        try
        {
            char *data = reinterpret_cast<char *>(packet_data->packet.get_data());
            socket.poll(packet_data->remote, data, packet_data->packet.get_mutable_size(), SS_MAX_PACKET_SIZE);

            if (packet_data->packet.get_size())
            {
                // TODO: check UDP checksum

                logger.push_event<LogProxy::LogLevel::Debug>("Received packet from ", packet_data->remote, " of size ", packet_data->packet.get_size(), " bytes...");

                workers.push(WorkerDecodeRequest(packet_data));

                packet_data = &packet_data_pool.alloc();
            }
        }
        catch (const UdpSocket::PollException &ex)
        {
            logger.push_event<LogProxy::LogLevel::Warning>(ex.what());
        }
    }
}

void Manager::resolve_server(const char *server_data, unsigned int server_size)
{
    Remote::Port port = SS_DEFAULT_PORT;

    // Extract port
    const char *port_i = server_data + server_size;
    unsigned int cur_port = 0;
    unsigned int digit_mul = 1;
    while (port_i > server_data)
    {
        char c = *--port_i;
        if (c >= '0' && c <= '9')
        {
            cur_port += (c - '0') * digit_mul;
            digit_mul *= 10;
        }
        else if (c == ':')
        {
            if (cur_port > 0 && cur_port < 65536)
            {
                port = cur_port;
            }
            server_size = port_i - server_data;
            break;
        }
        else
        {
            break;
        }
    }

    // Strip brackets from IPv6 addresses
    if (server_size && server_data[0] == '[' && server_data[server_size - 1] == ']')
    {
        server_data++;
        server_size -= 2;
    }

    Remote remote;

    try
    {
        remote.resolve(std::string(server_data, server_size), port);
    }
    catch (const std::exception &ex)
    {
        logger.push_event<LogProxy::LogLevel::Error>(ex.what());
        return;
    }

    //assert(!finalized_servers);

    // Lookup server connection corresponding to our remote
    ServerConnection *server_connection = find_server_connection(remote, true);
    assert(server_connection);
}

void Manager::worker_send_message(ServerConnection *server_connection, ServerConnection::InboxId inbox_id, const google::protobuf::Message *message)
{
    server_connection->send_message(inbox_id, *message);
}

void Manager::worker_func(WorkerRequest request)
{
    jw_util::Thread::assert_child_thread();

    if (request.is_resolve())
    {
        const WorkerResolveRequest resolve_request = request.to_resolve();
        resolve_server(resolve_request.get_data(), resolve_request.get_size());
        delete[] resolve_request.get_data();
    }
    else if (request.is_decode())
    {
        WorkerDecodeRequest::PacketData &decode_packet_data = *request.to_decode().get_packet_data();

        ServerConnection *server_connection = find_server_connection(decode_packet_data.remote, type == Type::Server);
        if (!server_connection)
        {
            logger.push_event<LogProxy::LogLevel::Notice>("Received message from unknown remote ", decode_packet_data.remote);
            return;
        }

        server_connection->receive_packet(decode_packet_data.packet);

        packet_data_pool.free(decode_packet_data);
    }
    else if (request.is_encode())
    {
        const WorkerEncodeRequest encode_request = request.to_encode();

        encode_request.get_message_router()->send_message(encode_request.get_inbox_id(), *encode_request.get_message());
    }
    else
    {
        assert(false);
    }
}

void Manager::interval_func()
{
    jw_util::Thread::assert_child_thread();

    std::chrono::steady_clock::time_point remote_threshold;
    remote_threshold = std::chrono::steady_clock::now();
    remote_threshold -= std::chrono::seconds(65);

    std::chrono::steady_clock::time_point self_ping_threshold;
    self_ping_threshold = std::chrono::steady_clock::now();
    self_ping_threshold -= std::chrono::seconds(30);

    std::chrono::steady_clock::time_point self_data_threshold;
    self_data_threshold = std::chrono::steady_clock::now();
    self_data_threshold -= std::chrono::milliseconds(5000);

    std::unordered_map<Remote, ServerConnection, Remote::Hasher>::iterator i = servers_map.begin();
    while (i != servers_map.end())
    {
        ServerConnection &server_connection = i->second;

        if (server_connection.is_remote_timed_out(remote_threshold))
        {
            // i = servers_map.erase(i);
        }
        else
        {
            std::chrono::steady_clock::time_point self_threshold = server_connection.has_data_to_send() ? self_data_threshold : self_ping_threshold;
            if (server_connection.is_self_timed_out(self_threshold))
            {
                server_connection.send_packet();
            }

            i++;
        }
    }
}

/*
void Manager::load_module(const std::string &interface, unsigned int version, const ModuleSpaceCoordinate *coords, unsigned int num_coords, jw_util::MethodCallback<ModuleInit> instantiate_callback)
{
    std::lock_guard<std::mutex> lock(loaded_modules_mutex);
    (void) lock;

    unsigned int module_id = loaded_modules.size();
    loaded_modules.emplace_back();
    LoadedModule &module = loaded_modules.back();

    module.message.set_module_id(module_id);
    module.message.set_interface(interface);
    module.message.set_version(version);

    for (unsigned int i = 0; i < num_coords; i++)
    {
        module.message.add_coords()->CopyFrom(coords[i]);
    }

    module.instantiate_callback = instantiate_callback;
}

void Manager::list_loaded_modules(ServerConnection &server_connection)
{
    std::lock_guard<std::mutex> lock(loaded_modules_mutex);
    (void) lock;

    std::vector<LoadedModule>::const_iterator i = loaded_modules.cbegin();
    while (i != loaded_modules.cend())
    {
        worker_func(WorkerEncodeRequest(&server_connection, ServerConnection::module_registration_inbox_id, &i->message));
        i++;
    }
}

void Manager::init_module_connection(ServerConnection &server_connection, const ModuleConnectionRequest &message)
{
    std::lock_guard<std::mutex> lock(loaded_modules_mutex);
    (void) lock;

    if (message.module_id() >= loaded_modules.size())
    {
        logger.push_event(LogProxy::LogLevel::Warning, "Received message from " + server_connection.get_remote().to_string() + " with invalid module_id " + std::to_string(message.module_id()));
        return;
    }

    LoadedModule &module = loaded_modules[message.module_id()];
    module.instantiate_callback.call(ModuleInit(server_connection, module.message.interface(), message.inbox_ids(), module.message.coords(), message.coords()));
}

void Manager::register_module(ServerConnection &server_connection, const ModuleRegistration &message)
{
#ifndef NDEBUG
    check_module_coords(message.coords());
#endif

    std::lock_guard<std::mutex> lock(register_module_mutex);
    (void) lock;

    // Insert module registration into registered_modules
    std::unordered_multimap<std::string, RegisteredModule>::iterator insert
            = registered_modules.emplace(std::piecewise_construct, std::forward_as_tuple(message.interface()), std::forward_as_tuple(server_connection, message.module_id(), message.coords()));

    // Sort the arrays so we can binary search them
    sort_by_key<std::vector<ModuleSpaceCoordinate>>(insert->second.coords);
}

Manager::RegisteredModule *Manager::lookup_module_local(ModuleLookupRequest &request, float &score)
{
    sort_by_key<google::protobuf::RepeatedPtrField<ModuleSpaceCoordinate>>(*request.mutable_coords());
    const google::protobuf::RepeatedPtrField<ModuleSpaceCoordinate> &req_coords = request.coords();

#ifndef NDEBUG
    check_module_coords(req_coords);
#endif

    float best_score = std::numeric_limits<float>::infinity();
    RegisteredModule *best_module = 0;

    std::lock_guard<std::mutex> lock(register_module_mutex);
    (void) lock;

    auto range = registered_modules.equal_range(request.interface());
    while (range.first != range.second)
    {
        float cur_score = 0.0f;

        const std::vector<ModuleSpaceCoordinate> &desc_coords = range.first->second.coords;
        google::protobuf::RepeatedPtrField<ModuleSpaceCoordinate>::const_iterator req_i = req_coords.cbegin();
        std::vector<ModuleSpaceCoordinate>::const_iterator desc_i = desc_coords.cbegin();

        while (req_i != req_coords.cend() && desc_i != desc_coords.cend())
        {
            if (req_i->key() < desc_i->key())
            {
                req_i++;
            }
            else if (req_i->key() > desc_i->key())
            {
                desc_i++;
            }
            else
            {
                assert(req_i->key() == desc_i->key());
                float weight = req_i->weight() * desc_i->weight();

                if (req_i->string_value().empty() && desc_i->string_value().empty())
                {
                    float error = req_i->scalar_value() - desc_i->scalar_value();
                    cur_score += weight * error * error;
                }
                else if (req_i->string_value() == desc_i->string_value())
                {
                    cur_score += weight;
                }
            }
        }

        if (cur_score < best_score)
        {
            best_score = cur_score;
            best_module = &range.first->second;
        }

        range.first++;
    }

    score = best_score;
    return best_module;
}
*/

/*
bool Manager::lookup_module(ModuleLookupRequest &request, const ModuleRegisterRequest *result_module, float &result_score) const
{
    const ModuleRegisterRequest *cur_module = 0;
    float cur_score = std::numeric_limits<float>::infinity();

    // Sort the arrays so we can binary search them
    sort_by_key<google::protobuf::RepeatedPtrField<ModuleLookupRequest_MatchEntry>>(*request.mutable_match());
    sort_by_key<google::protobuf::RepeatedPtrField<ModuleLookupRequest_MinimizeErrorsSqEntry>>(*request.mutable_minimize_errors_sq());

    auto range = registered_modules.equal_range(request.interface());
    while (range.first != range.second)
    {
        const ModuleRegisterRequest &desc = range.first->second;

        bool matches = lookup_module_matches(request.match(), desc.match());
        if (matches)
        {
            float score = lookup_module_score(request.minimize_errors_sq(), desc.minimize_errors_sq());
            if (score < cur_score)
            {
                cur_module = &desc;
                cur_score = score;
            }
        }

        range.first++;
    }

    if (cur_module)
    {
        result_module = cur_module;
        result_score = cur_score;
        return true;
    }
    else
    {
        return false;
    }
}

bool Manager::lookup_module_matches(const google::protobuf::RepeatedPtrField<ModuleLookupRequest_MatchEntry> &req_match, const google::protobuf::RepeatedPtrField<ModuleRegisterRequest_MatchEntry> &desc_match)
{
    google::protobuf::RepeatedPtrField<ModuleLookupRequest_MatchEntry>::const_iterator i1 = req_match.cbegin();
    google::protobuf::RepeatedPtrField<ModuleRegisterRequest_MatchEntry>::const_iterator i2 = desc_match.cbegin();

    while (i1 != req_match.cend() && i2 != desc_match.cend())
    {
        if (i1->key() < i2->key())
        {
            i1++;
        }
        else if (i1->key() > i2->key())
        {
            i2++;
        }
        else
        {
            assert(i1->key() == i2->key());
            if (i1->value() != i2->value()) {return false;}
        }
    }

    return true;
}

float Manager::lookup_module_score(const google::protobuf::RepeatedPtrField<ModuleLookupRequest_MinimizeErrorsSqEntry> &req_minimize_error_sq, const google::protobuf::RepeatedPtrField<ModuleRegisterRequest_MinimizeErrorsSqEntry> &desc_minimize_error_sq)
{
    float score = 0.0f;

    google::protobuf::RepeatedPtrField<ModuleLookupRequest_MinimizeErrorsSqEntry>::const_iterator i1 = req_minimize_error_sq.cbegin();
    google::protobuf::RepeatedPtrField<ModuleRegisterRequest_MinimizeErrorsSqEntry>::const_iterator i2 = desc_minimize_error_sq.cbegin();

    while (i1 != req_minimize_error_sq.cend() && i2 != desc_minimize_error_sq.cend())
    {
        if (i1->key() < i2->key())
        {
            i1++;
        }
        else if (i1->key() > i2->key())
        {
            i2++;
        }
        else
        {
            assert(i1->key() == i2->key());
            float weight = i1->weight();
            float error = i1->value() - i2->value();
            score += weight * error * error;
        }
    }

    return score;
}
*/

void Manager::log_handler(google::protobuf::LogLevel protobuf_level, const char *filename, int line, const std::string &protobuf_message)
{
    assert(instance);

    switch (protobuf_level)
    {
        case google::protobuf::LOGLEVEL_INFO: instance->logger.push_event<LogProxy::LogLevel::Notice>("Protobuf log from file \"", filename, "\" on line ", line, ": \"", protobuf_message, "\""); break;
        case google::protobuf::LOGLEVEL_WARNING: instance->logger.push_event<LogProxy::LogLevel::Warning>("Protobuf log from file \"", filename, "\" on line ", line, ": \"", protobuf_message, "\""); break;
        case google::protobuf::LOGLEVEL_ERROR: instance->logger.push_event<LogProxy::LogLevel::Error>("Protobuf log from file \"", filename, "\" on line ", line, ": \"", protobuf_message, "\""); break;
        case google::protobuf::LOGLEVEL_FATAL: instance->logger.push_event<LogProxy::LogLevel::Fatal>("Protobuf log from file \"", filename, "\" on line ", line, ": \"", protobuf_message, "\""); break;
        default: assert(false);
    }
}

}
