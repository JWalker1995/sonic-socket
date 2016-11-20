#ifndef MESSAGEDECODER_H
#define MESSAGEDECODER_H

#include <vector>

#include "libSonicSocket/jw_util/resizablestorage.h"

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

    bool extract_message(FountainCoder::DecodedPacket &decode);
    void clear_message();

    bool has_message() const;
    bool has_error() const;

    MessageMetaCompressor::Meta get_message_meta() const;
    const char *get_message_data() const;
    std::string get_error_string() const;

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
