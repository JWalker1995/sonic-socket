#include "catch/single_include/catch.hpp"
#include "libSonicSocket/workerrequest.h"
#include "libSonicSocket/base.pb.h"

TEST_CASE("WorkerResolveRequest", "") {
    const char *server_data = "example.com";
    static constexpr unsigned int server_size = sizeof(server_data);

    sonic_socket::WorkerRequest req = sonic_socket::WorkerResolveRequest(server_data, server_size);
    CHECK(req.is_resolve());
    CHECK_FALSE(req.is_decode());
    CHECK_FALSE(req.is_encode());
    CHECK(req.to_resolve().get_data() == server_data);
    CHECK(req.to_resolve().get_size() == server_size);
}

TEST_CASE("WorkerDecodeRequest", "") {
    sonic_socket::WorkerDecodeRequest::PacketData packet_data;

    sonic_socket::WorkerRequest req = sonic_socket::WorkerDecodeRequest(&packet_data);
    CHECK_FALSE(req.is_resolve());
    CHECK(req.is_decode());
    CHECK_FALSE(req.is_encode());
    CHECK(req.to_decode().get_packet_data() == &packet_data);
}

TEST_CASE("WorkerEncodeRequest", "") {
    char message_router_data[sizeof(sonic_socket::MessageRouter)];
    sonic_socket::MessageRouter *message_router = reinterpret_cast<sonic_socket::MessageRouter *>(message_router_data);
    sonic_socket::MessageRouter::InboxId inbox_id = 123;
    sonic_socket::Empty message;

    sonic_socket::WorkerRequest req = sonic_socket::WorkerEncodeRequest(message_router, inbox_id, &message);
    CHECK_FALSE(req.is_resolve());
    CHECK_FALSE(req.is_decode());
    CHECK(req.is_encode());
    CHECK(req.to_encode().get_message_router() == message_router);
    CHECK(req.to_encode().get_inbox_id() == inbox_id);
    CHECK(req.to_encode().get_message() == &message);
}
