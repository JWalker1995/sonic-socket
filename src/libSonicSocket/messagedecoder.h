#ifndef MESSAGEDECODER_H
#define MESSAGEDECODER_H

#include <vector>

#include "libSonicSocket/jw_util/resizablestorage.h"

#include "libSonicSocket/messageambigresolver.h"
#include "libSonicSocket/messagemetacompressor.h"
#include "libSonicSocket/fountaincoder.h"

namespace sonic_socket
{

class MessageDecoder
{
public:
    MessageDecoder()
        : recv_buffer(FountainBase::SymbolType::size + 1)
        , recv_ptr(recv_buffer.begin())
        , recv_bit_offset(0)
        , recv_state(RecvState::Init)
    {}

    bool extract_message(FountainCoder::DecodedPacket &decode) {
        while (decode.has_symbol())
        {
            FountainBase::SymbolType &symbol = decode.get_symbol();
            decode.next_symbol();

            bool ambig_low = symbol.is_ambig_low<MessageAmbigResolver::ambig_bits>();
            bool ambig_high = symbol.is_ambig_high<MessageAmbigResolver::ambig_bits>();

            switch (ambiguity_resolution)
            {
            case AmbiguityResolution::None:
                if (ambig_low || ambig_high) {
                    if (ambig_high) {
                        symbol.flip_ambiguity_low<MessageAmbigResolver::ambig_bits>();
                    }

                    unsigned int lsw = symbol.get_data()[0];
                    if (lsw == 0 || lsw == 1) {
                        ambiguity_resolution = lsw ? AmbiguityResolution::FlipHigh : AmbiguityResolution::FlipLow;
                        goto next_symbol;
                    } else {
                        recv_state = RecvState::ErrorUnresolvableAmbiguity;
                        return false;
                    }
                }
                break;

            case AmbiguityResolution::FlipLow:
                if (ambig_high) {
                    symbol.flip_ambiguity_low<MessageAmbigResolver::ambig_bits>();
                } else if (!ambig_low) {
                    recv_state = RecvState::ErrorExpectedAmbiguity;
                    return false;
                }
                ambiguity_resolution = AmbiguityResolution::None;
                break;

            case AmbiguityResolution::FlipHigh:
                if (ambig_low) {
                    symbol.flip_ambiguity_high<MessageAmbigResolver::ambig_bits>();
                } else if (!ambig_high) {
                    recv_state = RecvState::ErrorExpectedAmbiguity;
                    return false;
                }
                ambiguity_resolution = AmbiguityResolution::None;
                break;
            }

            symbol.write_to<true>(recv_ptr, recv_bit_offset);

            {
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

                        for (unsigned int i = recv_expecting; i < length; i++)
                        {
                            if (data[i] != 0x4A)
                            {
                                recv_state = RecvState::ErrorPaddingNotJ;
                                return false;
                            }
                        }

                        return true;
                    }
                    break;

                case RecvState::Pending:
                case RecvState::ErrorInvalidMeta:
                case RecvState::ErrorPaddingNotJ:
                case RecvState::ErrorUnresolvableAmbiguity:
                case RecvState::ErrorExpectedAmbiguity:
                    // Should never get here
                    assert(false);
                    break;
                }

                unsigned int recv_buffer_new_size = recv_ptr - recv_buffer.begin() + FountainBase::SymbolType::size + 1;
                recv_buffer.resize(recv_buffer_new_size, recv_ptr, recv_data);
            }

            next_symbol:;
        }

        return false;
    }

    void clear_message() {
        std::fill(recv_buffer.begin(), recv_ptr + 1, 0);

        recv_ptr = recv_buffer.begin();
        recv_bit_offset = 0;

        recv_state = RecvState::Init;
    }

    bool has_message() const {
        return recv_state == RecvState::Pending;
    }

    bool has_error() const {
        switch (recv_state)
        {
        case RecvState::ErrorInvalidMeta:
        case RecvState::ErrorPaddingNotJ:
        case RecvState::ErrorUnresolvableAmbiguity:
        case RecvState::ErrorExpectedAmbiguity:
            return true;
        default:
            return false;
        }
    }

    MessageMetaCompressor::Meta get_message_meta() const {
        assert(has_message());
        return recv_meta;
    }

    const char *get_message_data() const {
        assert(has_message());
        return reinterpret_cast<const char *>(recv_data);
    }

    std::string get_error_string() const {
        assert(has_error());

        switch (recv_state)
        {
        case RecvState::ErrorInvalidMeta:
            return "Invalid message meta data";
        case RecvState::ErrorPaddingNotJ:
            return "Padding after message is not Js (0x4A)";
        case RecvState::ErrorUnresolvableAmbiguity:
            return "Unresolvable ambiguous symbol";
        case RecvState::ErrorExpectedAmbiguity:
            return "Expected an ambiguity, but symbol was unambiguous";
        default:
            assert(false);
            return "";
        }
    }

private:
    enum class RecvState {Init, Pending, Meta, Message, ErrorInvalidMeta, ErrorPaddingNotJ, ErrorUnresolvableAmbiguity, ErrorExpectedAmbiguity};
    enum class AmbiguityResolution {None, FlipLow, FlipHigh};

    MessageMetaCompressor recv_meta_compressor;

    jw_util::ResizableStorage<mp_limb_t, true> recv_buffer;

    mp_limb_t *recv_ptr;
    unsigned int recv_bit_offset;

    RecvState recv_state;
    unsigned int recv_expecting;

    MessageMetaCompressor::Meta recv_meta;
    const unsigned char *recv_data;

    AmbiguityResolution ambiguity_resolution = AmbiguityResolution::None;
};

}

#endif // MESSAGEDECODER_H
