#include "catch/single_include/catch.hpp"
#include "libSonicSocket/messageencoder.h"
#include "libSonicSocket/messagedecoder.h"

TEST_CASE("MessageCoder", "")
{
    sonic_socket::LogProxy log_proxy_1;
    sonic_socket::FountainCoder coder(log_proxy_1);
    sonic_socket::MessageEncoder encoder;
    sonic_socket::MessageDecoder decoder;

    sonic_socket::MessageMetaCompressor::Meta meta;
    meta.inbox_id = 12345;
    meta.size = 23456;

    char *data = encoder.alloc_message(meta);
    for (unsigned int i = 0; i < 23456; i++) {
        data[i] = jw_util::Hash::combine(i * 11, i * 3);
    }

    encoder.send_message(coder);

    sonic_socket::FountainCoder::DecodedPacket packet(coder.get_unsent_symbols().begin(), coder.get_unsent_symbols().end());
    bool res = decoder.extract_message(packet);

    if (!res) {
        std::cout << decoder.get_error_string() << std::endl;
    }

    REQUIRE(res);
    REQUIRE(decoder.get_message_meta().inbox_id == meta.inbox_id);
    REQUIRE(decoder.get_message_meta().size == meta.size);

    REQUIRE(std::equal(data, data + 23456, decoder.get_message_data()));
}
