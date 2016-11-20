#include "messageencoder.h"

#include "messageambigresolver.h"

namespace sonic_socket
{

char *MessageEncoder::alloc_message(MessageMetaCompressor::Meta meta)
{
    assert(meta.inbox_id < 65536);
    assert(meta.size < 65536);

    MessageMetaCompressor::MetaEncoder encoder;
    encoder.meta = meta;
    send_meta_compressor.encode(encoder);
    unsigned int meta_length = send_meta_compressor.encoder_get_length(encoder);

    unsigned int send_size = meta_length + meta.size;
    send_symbols = jw_util::FastMath::div_ceil<unsigned int>(send_size * CHAR_BIT, FountainBase::SymbolType::modular_exponent);

    unsigned int limbs = jw_util::FastMath::div_ceil<unsigned int>(send_symbols * FountainBase::SymbolType::modular_exponent, sizeof(mp_limb_t) * CHAR_BIT);
    send_buffer.resize(limbs);

    for (unsigned int i = send_size / sizeof(mp_limb_t); i < limbs; i++)
    {
        send_buffer.begin()[i] = 0x4A4A4A4A4A4A4A4A;
    }

    unsigned char *data = reinterpret_cast<unsigned char *>(send_buffer.begin());
    send_meta_compressor.encoder_write_to(encoder, data);

    return reinterpret_cast<char *>(data + meta_length);
}

void MessageEncoder::send_message(FountainCoder &coder)
{
    const mp_limb_t *data = send_buffer.begin();
    unsigned int bit_offset = 0;

    /*
    std::cout << "Send: ";
    for (const unsigned char *i = reinterpret_cast<unsigned char *>(send_buffer.begin()); i < reinterpret_cast<unsigned char *>(send_buffer.end()); i++)
    {
        std::cout << static_cast<unsigned int>(*i) << " ";
    }
    std::cout << std::endl;
    */

    for (unsigned int i = 0; i < send_symbols; i++)
    {
        FountainBase::SymbolType &symbol = coder.alloc_symbol();
        symbol.read_from<true>(data, bit_offset);

        bool ambig_low = symbol.is_ambig_low<MessageAmbigResolver::ambig_bits>();
        bool ambig_high = symbol.is_ambig_high<MessageAmbigResolver::ambig_bits>();
        if (ambig_low || ambig_high) {
            FountainBase::SymbolType &next_symbol = coder.alloc_symbol();
            next_symbol = symbol;

            symbol = FountainBase::SymbolType(ambig_high);
            assert(symbol.is_ambig_low<MessageAmbigResolver::ambig_bits>());
        }
    }
}

}
