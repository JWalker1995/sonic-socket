#include "catch/single_include/catch.hpp"
#include "libSonicSocket/fountaincoder.h"

TEST_CASE("FountainCoder", "")
{
    sonic_socket::FountainBase::Packet packet;
    sonic_socket::FountainCoder::DecodedPacket decoded_packet;
    bool res;

    sonic_socket::LogProxy log_proxy_1;
    sonic_socket::FountainCoder coder_1(log_proxy_1);

    sonic_socket::LogProxy log_proxy_2;
    sonic_socket::FountainCoder coder_2(log_proxy_2);

    coder_1.alloc_symbol() = sonic_socket::FountainBase::SymbolType("123");
    coder_1.alloc_symbol() = sonic_socket::FountainBase::SymbolType("456");

    REQUIRE(coder_1.has_data_to_send());

    coder_1.generate_packet(packet);
    res = coder_2.recv_packet(packet, decoded_packet);
    REQUIRE(res == decoded_packet.has_symbol());

    REQUIRE(decoded_packet.has_symbol());
    REQUIRE(decoded_packet.get_symbol() == sonic_socket::FountainBase::SymbolType("123"));
    decoded_packet.next_symbol();

    REQUIRE(decoded_packet.has_symbol());
    REQUIRE(decoded_packet.get_symbol() == sonic_socket::FountainBase::SymbolType("456"));
    decoded_packet.next_symbol();

    REQUIRE(!decoded_packet.has_symbol());
}
