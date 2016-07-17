#include "messagedecoder.h"

namespace sonic_socket
{

bool MessageDecoder::extract_message(FountainCoder::DecodedPacket &decode)
{
    std::fill(recv_buffer.begin(), recv_ptr + 1, 0);

    recv_ptr = recv_buffer.begin();
    recv_bit_offset = 0;
    recv_state = RecvState::Init;

    while (decode.has_symbol())
    {
        FountainBase::SymbolType &symbol = decode.get_symbol();

        bool ambig_low = symbol.is_ambig_low();
        bool ambig_high = symbol.is_ambig_high();
        if (ambig_low || ambig_high)
        {
            assert(ambig_low ^ ambig_high);

            unsigned int max_decrement = recv_ptr - recv_buffer.begin();
            unsigned int actual_decrement = FountainBase::SymbolType::modular_exponent / GMP_LIMB_BITS;
            if (actual_decrement > max_decrement)
            {
                recv_state = RecvState::ErrorUnresolvableAmbiguity;
                return false;
            }

            FountainBase::SymbolType::seek<true, -1>(recv_ptr, recv_bit_offset);
            assert(recv_ptr >= recv_buffer.begin());

            const mp_limb_t *prev_ptr = recv_ptr;
            unsigned int prev_bit_offset = recv_bit_offset;
            FountainBase::SymbolType prev_symbol;
            prev_symbol.read_from<true>(prev_ptr, prev_bit_offset);

            FountainBase::SymbolType low_val(FountainBase::SymbolType::modular_decrement);
            FountainBase::SymbolType high_val(FountainBase::SymbolType::modular_decrement + 1);
            if (prev_symbol == low_val)
            {
                if (ambig_high) {symbol.flip_ambiguity_low();}
            }
            else if (prev_symbol == high_val)
            {
                if (ambig_low) {symbol.flip_ambiguity_high();}
            }
            else
            {
                recv_state = RecvState::ErrorInvalidAmbiguitySymbol;
                return false;
            }

            FountainBase::SymbolType::clear<true>(recv_ptr, recv_bit_offset);
        }

        symbol.write_to<true>(recv_ptr, recv_bit_offset);
        decode.next_symbol();

        const unsigned char *data = reinterpret_cast<const unsigned char *>(recv_buffer.begin());
        unsigned int length = (recv_ptr - recv_buffer.begin()) * sizeof(mp_limb_t) + (recv_bit_offset / CHAR_BIT);

        switch (recv_state)
        {
        case RecvState::Init:
            recv_state = RecvState::Meta;
            recv_expecting = recv_meta_compressor.decoder_get_length(data);
            // Fall-through intentional

        case RecvState::Meta:
            if (length >= recv_expecting)
            {
                bool success = recv_meta_compressor.decode(recv_meta, data);
                if (!success)
                {
                    /*
                    std::cout << "Recv invalid: ";
                    for (const unsigned char *i = reinterpret_cast<unsigned char *>(recv_buffer.begin()); i < reinterpret_cast<unsigned char *>(recv_buffer.end()); i++)
                    {
                        std::cout << static_cast<unsigned int>(*i) << " ";
                    }
                    std::cout << std::endl;
                    */

                    recv_state = RecvState::ErrorInvalidMeta;
                    return false;
                }

                recv_state = RecvState::Message;
                recv_data = data + recv_expecting;
                recv_expecting += recv_meta.size;
                // Fall-through intentional
            }
            else
            {
                break;
            }

        case RecvState::Message:
            if (length >= recv_expecting)
            {
                recv_state = RecvState::Pending;

                /*
                std::cout << "Recv: ";
                for (const unsigned char *i = reinterpret_cast<unsigned char *>(recv_buffer.begin()); i < reinterpret_cast<unsigned char *>(recv_buffer.end()); i++)
                {
                    std::cout << static_cast<unsigned int>(*i) << " ";
                }
                std::cout << std::endl;
                */

                for (unsigned int i = recv_expecting; i <= length; i++)
                {
                    if (data[i] != 0)
                    {
                        recv_state = RecvState::ErrorPaddingNonzero;
                        return false;
                    }
                }

                return true;
            }
            break;

        case RecvState::Pending:
        case RecvState::ErrorInvalidMeta:
        case RecvState::ErrorPaddingNonzero:
        case RecvState::ErrorUnresolvableAmbiguity:
        case RecvState::ErrorInvalidAmbiguitySymbol:
            // Should never get here
            assert(false);
            break;
        }

        unsigned int recv_buffer_new_size = recv_ptr - recv_buffer.begin() + FountainBase::SymbolType::size + 1;
        recv_buffer.resize(recv_buffer_new_size, recv_ptr, recv_data);
    }

    return false;
}

bool MessageDecoder::has_message() const
{
    return recv_state == RecvState::Pending;
}

bool MessageDecoder::has_error() const
{
    switch (recv_state)
    {
    case RecvState::ErrorInvalidMeta:
    case RecvState::ErrorPaddingNonzero:
    case RecvState::ErrorUnresolvableAmbiguity:
    case RecvState::ErrorInvalidAmbiguitySymbol:
        return true;
    default:
        return false;
    }
}

MessageMetaCompressor::Meta MessageDecoder::get_message_meta() const
{
    assert(has_message());
    return recv_meta;
}

const char *MessageDecoder::get_message_data() const
{
    assert(has_message());
    return reinterpret_cast<const char *>(recv_data);
}

std::string MessageDecoder::get_error_string() const
{
    assert(has_error());

    switch (recv_state)
    {
    case RecvState::ErrorInvalidMeta:
        return "Invalid message meta data";
    case RecvState::ErrorPaddingNonzero:
        return "Padding after message is not zero";
    case RecvState::ErrorUnresolvableAmbiguity:
        return "Unresolvable ambiguous symbol";
    case RecvState::ErrorInvalidAmbiguitySymbol:
        return "Invalid ambiguity resolution symbol";
    default:
        assert(false);
        return "";
    }
}

}
