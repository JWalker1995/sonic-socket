#ifndef MESSAGEMETACOMPRESSOR_H
#define MESSAGEMETACOMPRESSOR_H

#include <climits>

#include "libSonicSocket/config/JWUTIL_CACHELRU_FORGET_POOL_ON_HEAP.h"
#include "libSonicSocket/jw_util/cachelru.h"
#include "libSonicSocket/jw_util/hash.h"

namespace sonic_socket
{

class MessageMetaCompressor
{
public:
    struct Meta
    {
        unsigned int inbox_id;
        unsigned int size;

        bool operator==(const Meta &other) const
        {
            return inbox_id == other.inbox_id && size == other.size;
        }

        struct Hasher
        {
            std::size_t operator()(const Meta &obj) const
            {
                std::size_t hash = 0;
                hash = jw_util::Hash::combine(hash, obj.inbox_id);
                hash = jw_util::Hash::combine(hash, obj.size);
                return hash;
            }
        };
    };

private:
    static constexpr unsigned int cache_size = (1 << CHAR_BIT) - 1;
    static_assert(cache_size == 255, "Unexpected cache_size");

    struct Empty {};
    typedef jw_util::CacheLRU<Meta, Empty, cache_size, Meta::Hasher> CacheType;

public:
    class MetaEncoder
    {
        friend class MessageMetaCompressor;

    public:
        Meta meta;

    private:
        CacheType::Result cache_result;
    };

    void encode(MetaEncoder &encoder)
    {
        assert(encoder.meta.inbox_id < 65536);
        assert(encoder.meta.size < 65536);

        encoder.cache_result = cache.access(encoder.meta);
    }
    unsigned int encoder_get_length(const MetaEncoder &encoder) const
    {
        return encoder.cache_result.is_valid() ? 1 : 5;
    }
    void encoder_write_to(const MetaEncoder &encoder, char *data) const
    {
        if (encoder.cache_result.is_valid())
        {
            unsigned int id = cache.get_bucket_id(encoder.cache_result);
            assert(id < cache_size);
            *data++ = id;
        }
        else
        {
            *data++ = cache_size;

            *data++ = (encoder.meta.inbox_id >> 0) & 0xFF;
            *data++ = (encoder.meta.inbox_id >> 8) & 0xFF;

            *data++ = (encoder.meta.size >> 0) & 0xFF;
            *data++ = (encoder.meta.size >> 8) & 0xFF;
        }
    }

    unsigned int decoder_get_length(const char *data) const
    {
        return *data == static_cast<char>(cache_size) ? 5 : 1;
    }
    bool decode(Meta &res, const char *data)
    {
        if (*data == static_cast<char>(cache_size))
        {
            data++;

            res.inbox_id = 0;
            res.inbox_id |= *data++ >> 0;
            res.inbox_id |= *data++ >> 8;

            res.size = 0;
            res.size |= *data++ >> 0;
            res.size |= *data++ >> 8;
        }
        else
        {
            unsigned int id = static_cast<unsigned char>(*data++);
            if (!cache.is_bucket_id_valid(id)) {return false;}
            res = cache.lookup_bucket(id);
        }

        cache.access(res);
        return true;
    }

    static constexpr unsigned int max_encode_length = 5;

private:
    CacheType cache;
};

}

#endif // MESSAGEMETACOMPRESSOR_H
