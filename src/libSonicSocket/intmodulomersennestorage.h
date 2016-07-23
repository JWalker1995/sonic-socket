#ifndef INTMODULOMERSENNESTORAGE_H
#define INTMODULOMERSENNESTORAGE_H

#include "libSonicSocket/intmodulomersenne.h"

namespace sonic_socket
{

template <unsigned int modular_exponent_, unsigned int modular_decrement_>
class IntModuloMersenneStorage : public IntModuloMersenne<modular_exponent_, modular_decrement_>
{
    /*
    static constexpr unsigned int modular_exponent = modular_exponent_;
    static constexpr unsigned int modular_decrement = modular_decrement_;

    typedef IntModuloMersenne<modular_exponent, modular_decrement> ThisType;

    // The number of data elements needed
    static constexpr unsigned int size = (modular_exponent / GMP_NUMB_BITS) + (modular_exponent % GMP_NUMB_BITS != 0);

    // Head refers to the most significant data element
    static constexpr unsigned int head_bits = ((modular_exponent - 1) % GMP_NUMB_BITS) + 1;
    static constexpr mp_limb_t head_mask = (~static_cast<mp_limb_t>(0)) >> (GMP_LIMB_BITS - head_bits);

    static_assert(size * GMP_NUMB_BITS - (GMP_NUMB_BITS - head_bits) == modular_exponent, "Unexpected number of bits somewhere");
    static_assert((head_mask & ~GMP_NUMB_MASK) == 0, "Unexpected head_mask");
    */

public:
    typedef IntModuloMersenne<modular_exponent_, modular_decrement_> BaseType;

private:

    using BaseType::head_bits;
    using BaseType::head_mask;
    using BaseType::data;

public:
    using BaseType::modular_decrement;
    using BaseType::modular_exponent;
    using BaseType::size;

    IntModuloMersenneStorage()
        : BaseType()
    {}

    IntModuloMersenneStorage(mp_limb_t val)
        : BaseType(val)
    {}

    IntModuloMersenneStorage(const std::string &str)
        : BaseType(str)
    {}

    IntModuloMersenneStorage(const BaseType &other)
        : BaseType(other)
    {}

    template <bool bit_offset_starts_zero>
    void read_from(const mp_limb_t *&src, unsigned int &bit_offset)
    {
        static constexpr bool skip_shift = can_skip_shift<bit_offset_starts_zero>();

        for (unsigned int i = 0; i < size - 1; i++)
        {
            data[i] = read_bits<GMP_NUMB_BITS, skip_shift>(src, bit_offset);
        }

        data[size - 1] = read_bits<head_bits, skip_shift>(src, bit_offset);
    }

    template <bool bit_offset_starts_zero>
    void write_to(mp_limb_t *&dst, unsigned int &bit_offset) const
    {
        static constexpr bool skip_shift = can_skip_shift<bit_offset_starts_zero>();

        for (unsigned int i = 0; i < size - 1; i++)
        {
            write_bits<GMP_NUMB_BITS, skip_shift>(dst, bit_offset, data[i]);
        }

        write_bits<head_bits, skip_shift>(dst, bit_offset, data[size - 1]);
    }

    template <bool bit_offset_starts_zero>
    static void clear(mp_limb_t *dst, unsigned int bit_offset)
    {
        static constexpr bool skip_shift = can_skip_shift<bit_offset_starts_zero>();

        for (unsigned int i = 0; i < size - 1; i++)
        {
            clear_bits<GMP_NUMB_BITS, skip_shift>(dst, bit_offset);
        }

        clear_bits<head_bits, skip_shift>(dst, bit_offset);
    }

    template <bool bit_offset_starts_zero, signed int offset>
    static void seek(mp_limb_t *&ptr, unsigned int &bit_offset)
    {
        static constexpr bool skip_shift = can_skip_shift<bit_offset_starts_zero>();
        static constexpr signed int offset_bits = modular_exponent * offset;
        static constexpr signed int ptr_inc = (offset_bits < 0) ? -((-offset_bits) / GMP_LIMB_BITS) : (offset_bits / GMP_LIMB_BITS);
        static constexpr signed int bit_offset_inc = offset_bits - ptr_inc * GMP_LIMB_BITS;

        ptr += ptr_inc;

        if (skip_shift)
        {
            assert(bit_offset == 0);
            assert(bit_offset_inc == 0);
        }
        else
        {
            bit_offset += bit_offset_inc;
        }
    }

    void set_bit(unsigned int index, bool value = true)
    {
        assert(index < modular_exponent);
        assert((data[index / GMP_NUMB_BITS] & static_cast<mp_limb_t>(1) << (index % GMP_NUMB_BITS)) == 0);
        data[index / GMP_NUMB_BITS] |= static_cast<mp_limb_t>(value) << (index % GMP_NUMB_BITS);
    }

    bool get_bit(unsigned int index)
    {
        assert(index < modular_exponent);
        return (data[index / GMP_NUMB_BITS] >> (index % GMP_NUMB_BITS)) & 1;
    }

    bool is_ambig_low() const
    {
        if (data[0] >= modular_decrement) {return false;}
        for (unsigned int i = 1; i < size; i++)
        {
            if (data[i] != 0) {return false;}
        }
        return true;
    }

    bool is_ambig_high() const
    {
        if (modular_decrement == 0) {return false;}

        if (size == 1)
        {
            return data[0] > head_mask - modular_decrement;
        }
        else
        {
            if (data[0] < -static_cast<mp_limb_t>(modular_decrement)) {return false;}
            for (unsigned int i = 1; i < size - 1; i++)
            {
                if (data[i] != ~static_cast<mp_limb_t>(0)) {return false;}
            }
            if (data[size - 1] != head_mask) {return false;}
            return true;
        }
    }

    void flip_ambiguity_low()
    {
        assert(is_ambig_high());

        data[0] += modular_decrement;
        for (unsigned int i = 1; i < size; i++)
        {
            data[i] = 0;
        }

        assert(is_ambig_low());
    }

    void flip_ambiguity_high()
    {
        assert(is_ambig_low());

        data[0] -= modular_decrement;
        if (size == 1)
        {
            data[0] &= head_mask;
        }
        else
        {
            for (unsigned int i = 1; i < size - 1; i++)
            {
                data[i] = ~static_cast<mp_limb_t>(0);
            }
            data[size - 1] = head_mask;
        }

        assert(is_ambig_high());
    }

private:
    template <bool bit_offset_starts_zero>
    static constexpr bool can_skip_shift()
    {
        return bit_offset_starts_zero && GMP_NUMB_BITS == GMP_LIMB_BITS && head_bits == GMP_LIMB_BITS;
    }

    template <unsigned int advance, bool skip_shift>
    static mp_limb_t read_bits(const mp_limb_t *&src, unsigned int &bit_offset)
    {
        assert(bit_offset < GMP_LIMB_BITS);

        if (skip_shift)
        {
            assert(bit_offset == 0);
            assert(advance == GMP_LIMB_BITS);
            return *src++;
        }

        mp_limb_t limb = src[0] >> bit_offset;
        if (bit_offset > GMP_LIMB_BITS - advance)
        {
            limb |= src[1] << (GMP_LIMB_BITS - bit_offset);
        }

        if (advance == GMP_LIMB_BITS)
        {
            src++;
        }
        else
        {
            bit_offset += advance;
            if (bit_offset >= GMP_LIMB_BITS)
            {
                bit_offset -= GMP_LIMB_BITS;
                src++;
            }

            static constexpr mp_limb_t mask = (~static_cast<mp_limb_t>(0)) >> (GMP_LIMB_BITS - advance);
            limb &= mask;
        }

        return limb;
    }

    template <unsigned int advance, bool skip_shift>
    static void write_bits(mp_limb_t *&dst, unsigned int &bit_offset, mp_limb_t limb)
    {
        assert(bit_offset < GMP_LIMB_BITS);

        if (skip_shift)
        {
            assert(bit_offset == 0);
            assert(advance == GMP_LIMB_BITS);
            *dst++ = limb;
            return;
        }

        if (advance != GMP_LIMB_BITS)
        {
            assert(limb < static_cast<mp_limb_t>(2) << (advance - 1));
        }

        dst[0] |= limb << bit_offset;
        if (bit_offset > GMP_LIMB_BITS - advance)
        {
            dst[1] |= limb >> (GMP_LIMB_BITS - bit_offset);
        }

        if (advance == GMP_LIMB_BITS)
        {
            dst++;
        }
        else
        {
            bit_offset += advance;
            if (bit_offset >= GMP_LIMB_BITS)
            {
                bit_offset -= GMP_LIMB_BITS;
                dst++;
            }
        }
    }

    template <unsigned int advance, bool skip_shift>
    static void clear_bits(mp_limb_t *&dst, unsigned int &bit_offset)
    {
        assert(bit_offset < GMP_LIMB_BITS);

        if (skip_shift)
        {
            assert(bit_offset == 0);
            assert(advance == GMP_LIMB_BITS);
            *dst++ = 0;
            return;
        }

        if (bit_offset)
        {
            dst[0] &= (~static_cast<mp_limb_t>(0)) >> (GMP_LIMB_BITS - bit_offset);
            dst[1] &= (~static_cast<mp_limb_t>(0)) << bit_offset;
        }
        else
        {
            dst[0] = 0;
        }

        if (advance == GMP_LIMB_BITS)
        {
            dst++;
        }
        else
        {
            bit_offset += advance;
            if (bit_offset >= GMP_LIMB_BITS)
            {
                bit_offset -= GMP_LIMB_BITS;
                dst++;
            }
        }
    }
};

}

#endif // INTMODULOMERSENNESTORAGE_H
