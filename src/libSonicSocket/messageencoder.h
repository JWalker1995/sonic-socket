#ifndef MESSAGEENCODER_H
#define MESSAGEENCODER_H

#include "libSonicSocket/jw_util/resizablestorage.h"

#include "libSonicSocket/messagemetacompressor.h"
#include "libSonicSocket/fountaincoder.h"

namespace sonic_socket
{

class MessageEncoder
{
public:
    MessageEncoder()
        : send_buffer(16)
    {}

    char *alloc_message(MessageMetaCompressor::Meta message);
    void send_message(FountainCoder &coder);

private:
    MessageMetaCompressor send_meta_compressor;

    jw_util::ResizableStorage<mp_limb_t> send_buffer;

    unsigned int send_symbols;
};

}

#endif // MESSAGEENCODER_H
