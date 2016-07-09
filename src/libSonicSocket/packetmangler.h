#ifndef PACKETMANGLER_H
#define PACKETMANGLER_H

#include <assert.h>
#include <limits.h>

template <typename DataType>
class PacketMangler
{
public:
    PacketMangler() = delete;

    template <unsigned int passes = 2>
    static void mangle(DataType *data, unsigned int offset_bytes, unsigned int limit_bytes)
    {
        transform<false, passes>(data, offset_bytes, limit_bytes);
    }

    template <unsigned int passes = 2>
    static void demangle(DataType *data, unsigned int offset_bytes, unsigned int limit_bytes)
    {
        transform<true, passes>(data, offset_bytes, limit_bytes);
    }

private:
    template <bool forward, unsigned int passes>
    static void transform(DataType *data, unsigned int offset_bytes, unsigned int limit_bytes)
    {
        assert(offset_bytes < sizeof(DataType));

        if (limit_bytes - offset_bytes < 2) {return;}

        unsigned int words = limit_bytes / sizeof(DataType);
        if (words < 3 || sizeof(DataType) == 1)
        {
            unsigned char *data_bytes = reinterpret_cast<unsigned char *>(data);
            unsigned char *data_bytes_begin = data_bytes + offset_bytes;
            unsigned char *data_bytes_end = data_bytes + limit_bytes;

            for (unsigned int i = 0; i < passes; i++)
            {
                if (forward)
                {
                    for (unsigned char *j = data_bytes_begin; ++j != data_bytes_end; )
                    {
                        mangle_aligned(j - 1, j);
                    }
                    mangle_aligned(data_bytes_end - 1, data_bytes_begin);
                }
                else
                {
                    mangle_aligned(data_bytes_end - 1, data_bytes_begin);
                    for (unsigned char *j = data_bytes_end; --j != data_bytes_begin; )
                    {
                        mangle_aligned(j - 1, j);
                    }
                }
            }
            return;
        }

        unsigned int offset_bits = offset_bytes * CHAR_BIT;
        unsigned int limit_bits = (limit_bytes % sizeof(DataType)) * CHAR_BIT;

        for (unsigned int i = 0; i < passes; i++)
        {
            if (forward)
            {
                DataType *cur = data;

                mangle_unaligned(cur, offset_bits, cur + 1, offset_bits);
                cur++;

                for (unsigned int j = 2; j < words; j++)
                {
                    mangle_aligned(cur, cur + 1);
                    cur++;
                }

                if (limit_bits)
                {
                    mangle_unaligned(cur - 1, limit_bits, cur, limit_bits);
                }

                mangle_unaligned(cur, limit_bits, data, offset_bits);

                assert(cur == data + words - 1);
            }
            else
            {
                DataType *cur = data + words - 1;

                mangle_unaligned(cur, limit_bits, data, offset_bits);

                if (limit_bits)
                {
                    mangle_unaligned(cur - 1, limit_bits, cur, limit_bits);
                }

                for (unsigned int j = 2; j < words; j++)
                {
                    cur--;
                    mangle_aligned(cur, cur + 1);
                }

                cur--;
                mangle_unaligned(cur, offset_bits, cur + 1, offset_bits);

                assert(cur == data);
            }
        }
    }

    template <typename ElementType>
    static void mangle_unaligned(ElementType *src, unsigned int src_bit_offset, ElementType *dst, unsigned int dst_bit_offset)
    {
        assert(dst_bit_offset < sizeof(ElementType) * CHAR_BIT);
        assert(src_bit_offset < sizeof(ElementType) * CHAR_BIT);

        ElementType val = 0;
        val |= src[0] >> src_bit_offset;
        if (src_bit_offset)
        {
            val |= src[1] << (sizeof(ElementType) * CHAR_BIT - src_bit_offset);
        }

        val = hash(val);

        dst[0] ^= val << dst_bit_offset;
        if (dst_bit_offset)
        {
            dst[1] ^= val >> (sizeof(ElementType) * CHAR_BIT - dst_bit_offset);
        }
    }

    template <typename ElementType>
    static void mangle_aligned(ElementType *src, ElementType *dst)
    {
        *dst ^= hash(*src);
    }

    template <typename ElementType>
    static ElementType hash(ElementType key)
    {
        key = (~key) + (key << 21); // key = (key << 21) - key - 1;
        key ^= (key >> 24);
        key += (key << 3) + (key << 8); // key * 265
        key ^= (key >> 14);
        key += (key << 2) + (key << 4); // key * 21
        key ^= (key >> 28);
        key += (key << 31);
        return key;

        /*
        // Based on MurmurHash2: https://sites.google.com/site/murmurhash/

        static constexpr ElementType mul = static_cast<ElementType>(0xC6A4A7935BD1E995ull);
        static constexpr ElementType add = static_cast<ElementType>(0xC9C58D1D9C4F65BEull);
        static constexpr unsigned int r = 47;

        key *= mul;
        key ^= key >> r;
        key *= mul;
        key += add;

        return key;
        */
    }
};

#endif // PACKETMANGLER_H
