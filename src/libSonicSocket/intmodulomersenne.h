#ifndef INTMODULOMERSENNE_H
#define INTMODULOMERSENNE_H

#include <algorithm>
#include <assert.h>

#include "gmp.h"

#include "libSonicSocket/config/JWUTIL_CACHELRU_FORGET_POOL_ON_HEAP.h"
#include "libSonicSocket/jw_util/cachelru.h"
#include "libSonicSocket/jw_util/fastmath.h"
#include "libSonicSocket/jw_util/hash.h"

#include "libSonicSocket/config/SS_INTMODULOMERSENNE_INVERSE_CACHE_SIZE.h"

// Integers n such that 2^n - 1 is prime: (http://oeis.org/A000043)
// unsigned int mersenne_primes[] = {2, 3, 5, 7, 13, 17, 19, 31, 61, 89, 107, 127, 521, 607, 1279, 2203, 2281, 3217, 4253, 4423, 9689, 9941, 11213, 19937, 21701, 23209, 44497, 86243, 110503, 132049, 216091, 756839, 859433, 1257787, 1398269, 2976221, 3021377, 6972593, 13466917, 20996011, 24036583, 25964951, 30402457, 32582657};
// Actually, don't even need mersenne primes, just primes close to a power of 2:
// https://oeis.org/A013603


namespace sonic_socket
{

template <unsigned int modular_exponent_, unsigned int modular_decrement_>
class IntModuloMersenne
{
    static_assert(modular_decrement_ != 0, "modular_decrement must be positive");

public:
    static constexpr unsigned int modular_exponent = modular_exponent_;
    static constexpr unsigned int modular_decrement = modular_decrement_;

protected:
    typedef IntModuloMersenne<modular_exponent, modular_decrement> ThisType;

    // The number of data elements needed
    static constexpr unsigned int size = jw_util::FastMath::div_ceil<unsigned int>(modular_exponent, GMP_NUMB_BITS);

    // Head refers to the most significant data element
    static constexpr unsigned int head_bits = ((modular_exponent - 1) % GMP_NUMB_BITS) + 1;
    static constexpr unsigned int unused_bits = GMP_NUMB_BITS - head_bits;
    static constexpr mp_limb_t head_mask = (~static_cast<mp_limb_t>(0)) >> (GMP_LIMB_BITS - head_bits);

    static_assert(size * GMP_NUMB_BITS - unused_bits == modular_exponent, "Unexpected number of bits somewhere");
    static_assert((head_mask & ~GMP_NUMB_MASK) == 0, "Unexpected head_mask");

public:
    IntModuloMersenne()
    {
#ifndef NDEBUG
        mpn_random(data, size);
        data[size - 1] &= head_mask;
#endif
    }

    IntModuloMersenne(mp_limb_t val)
    {
        assert((val & ~GMP_NUMB_MASK) == 0);

        if (size == 1)
        {
            assert((val & ~head_mask) == 0);
        }

        data[0] = val;
        std::fill(data + 1, data + size, 0);
    }

    IntModuloMersenne(const std::string &str)
    {
        from_string<10>(str);
    }

    IntModuloMersenne(const ThisType &other)
    {
        operator=(other);
    }

    ThisType &operator=(const ThisType &other)
    {
        std::copy(other.data, other.data + size, data);
        return *this;
    }


    ThisType operator+(const ThisType &other) const
    {
        ThisType res;
        add(res, *this, other);
        return res;
    }
    ThisType &operator+=(const ThisType &other)
    {
        add(*this, *this, other);
        return *this;
    }

    ThisType operator-(const ThisType &other) const
    {
        ThisType res;
        sub(res, *this, other);
        return res;
    }
    ThisType &operator-=(const ThisType &other)
    {
        sub(*this, *this, other);
        return *this;
    }

    ThisType operator*(const ThisType &other) const
    {
        ThisType res;
        mul(res, *this, other);
        return res;
    }
    ThisType &operator*=(const ThisType &other)
    {
        mul(*this, *this, other);
        return *this;
    }

    ThisType operator/(const ThisType &other) const
    {
        ThisType res;
        mul(res, *this, *other.inverse());
        return res;
    }

    ThisType &operator/=(const ThisType &other)
    {
        mul(*this, *this, *other.inverse());
        return *this;
    }

    template <bool use_cache = true>
    ThisType *inverse() const
    {
        // This method returns a pointer to a temporary element.
        // When this method is called again, all previous returned values become invalid.

        typedef jw_util::CacheLRU<ThisType, ThisType, SS_INTMODULOMERSENNE_INVERSE_CACHE_SIZE, Hasher> CacheType;

        typename CacheType::Result res;
        if (use_cache)
        {
            static CacheType cache;
            res = cache.access(*this);

            if (!res.is_valid())
            {
                calc_inverse(*res.get_value(), *this);
            }

            return res.get_value();
        }
        else
        {
            static ThisType tmp;
            calc_inverse(tmp, *this);
            return &tmp;
        }
    }


    template <unsigned int ambig_values = modular_decrement>
    bool is_ambig_low() const
    {
        if (data[0] >= ambig_values) {return false;}
        for (unsigned int i = 1; i < size; i++)
        {
            if (data[i] != 0) {return false;}
        }
        return true;
    }

    template <unsigned int ambig_values = modular_decrement>
    bool is_ambig_high() const
    {
        if (ambig_values == 0) {return false;}

        if (size == 1)
        {
            return data[0] > head_mask - ambig_values;
        }
        else
        {
            if (data[0] < -static_cast<mp_limb_t>(ambig_values)) {return false;}
            for (unsigned int i = 1; i < size - 1; i++)
            {
                if (data[i] != ~static_cast<mp_limb_t>(0)) {return false;}
            }
            if (data[size - 1] != head_mask) {return false;}
            return true;
        }
    }

    template <unsigned int ambig_values = modular_decrement>
    void flip_ambiguity_low()
    {
        assert(is_ambig_high<ambig_values>());
        set_ambiguity<0>(data, data[0] + ambig_values);
        assert(is_ambig_low<ambig_values>());
    }

    template <unsigned int ambig_values = modular_decrement>
    void flip_ambiguity_high()
    {
        assert(is_ambig_low<ambig_values>());
        set_ambiguity<~static_cast<mp_limb_t>(0)>(data, data[0] - ambig_values);
        assert(is_ambig_high<ambig_values>());
    }


    template <bool resolve_ambiguities>
    static int cmp(const ThisType &a, const ThisType &b)
    {
        if (resolve_ambiguities)
        {
            bool a_high = a.is_ambig_high();
            bool b_high = b.is_ambig_high();
            if (a_high && !b_high)
            {
                mp_limb_t a_mut[size];
                set_ambiguity<0>(a_mut, a.data[0] + modular_decrement);
                return mpn_cmp(a_mut, b.data, size);
            }
            else if (b_high && !a_high)
            {
                mp_limb_t b_mut[size];
                set_ambiguity<0>(b_mut, b.data[0] + modular_decrement);
                return mpn_cmp(a.data, b_mut, size);
            }
            else
            {
                return mpn_cmp(a.data, b.data, size);
            }
        }
        else
        {
            return mpn_cmp(a.data, b.data, size);
        }
    }

    bool operator==(const ThisType &other) const
    {
        return cmp<true>(*this, other) == 0;
    }
    bool operator!=(const ThisType &other) const
    {
        return cmp<true>(*this, other) != 0;
    }
    bool operator<(const ThisType &other) const
    {
        return cmp<true>(*this, other) < 0;
    }
    bool operator<=(const ThisType &other) const
    {
        return cmp<true>(*this, other) <= 0;
    }
    bool operator>(const ThisType &other) const
    {
        return cmp<true>(*this, other) > 0;
    }
    bool operator>=(const ThisType &other) const
    {
        return cmp<true>(*this, other) >= 0;
    }


    template <unsigned int base = 10>
    std::string to_string() const
    {
        static_assert(base >= 2, "Base is too low");
        static_assert(base <= 256, "Base is too high");

#include "log2s.h"
        static constexpr unsigned int max_digits = GMP_NUMB_BITS * size * log2s[base] + 2;

        mp_limb_t tmp[size];
        std::copy(data, data + size, tmp);

        unsigned int s = size;
        while (s > 0 && tmp[s - 1] == 0)
        {
            s--;
        }

        if (s == 0)
        {
            return "0";
        }

        unsigned char str[max_digits];
        unsigned int end = mpn_get_str(str, base, tmp, s);

        unsigned int start = 0;
        while (start < end && str[start] == 0)
        {
            start++;
        }

        unsigned int digits = end - start;

        assert(digits > 0);

        std::string res;
        res.resize(digits);
        for (unsigned int i = 0; i < digits; i++)
        {
            unsigned char d = str[start + i];
            if (d < 10)
            {
                res[i] = '0' + d;
            }
            else
            {
                res[i] = 'A' + (d - 10);
            }
        }

        return res;
    }

    template <unsigned int base = 10>
    void from_string(const std::string &str)
    {
        static_assert(base >= 2, "Base is too low");
        static_assert(base <= 256, "Base is too high");

        if (str.empty())
        {
            std::fill_n(data, size, 0);
        }

        unsigned char *bytes = new unsigned char[str.size()];
        for (unsigned int i = 0; i < str.size(); i++)
        {
            if (str[i] >= '0' && str[i] <= '9')
            {
                bytes[i] = str[i] - '0';
            }
            else if (str[i] >= 'A' && str[i] <= 'Z')
            {
                bytes[i] = str[i] - 'A' + 10;
            }
            else
            {
                assert(false);
            }

            assert(bytes[i] < base);
        }

        mp_limb_t dst[size + 1];
        unsigned int words = mpn_set_str(dst, bytes, str.size(), base);
        assert(words <= size);

        std::copy_n(dst, words, data);
        for (unsigned int i = words; i < size; i++)
        {
            data[i] = 0;
        }
    }

    mp_limb_t *get_data() {return data;}

    struct Hasher
    {
        std::size_t operator()(const ThisType &num) const
        {
            std::size_t res = 0;

            for (unsigned int i = 0; i < size; i++)
            {
                res = jw_util::Hash::combine(res, num.data[i]);
            }

            return res;
        }
    };

protected:
    mp_limb_t data[size];

private:
    static void add(ThisType &res, const ThisType &a, const ThisType &b)
    {
        mp_limb_t carry = mpn_add_n(res.data, a.data, b.data, size);

        if (head_bits != GMP_NUMB_BITS)
        {
            assert(carry == 0);
            carry = res.data[size - 1] >> (head_bits % GMP_LIMB_BITS);
            res.data[size - 1] &= head_mask;
        }

        assert(carry == 0 || carry == 1);
        if (carry)
        {
            mpn_add_1(res.data, res.data, size, modular_decrement);
        }
    }

    static void sub(ThisType &res, const ThisType &a, const ThisType &b)
    {
        mp_limb_t borrow = mpn_sub_n(res.data, a.data, b.data, size);

        if (head_bits != GMP_NUMB_BITS)
        {
            assert(borrow == ((res.data[size - 1] >> (head_bits % GMP_LIMB_BITS)) & 1));
            res.data[size - 1] &= head_mask;
        }

        assert(borrow == 0 || borrow == 1);
        if (borrow)
        {
            mpn_sub_1(res.data, res.data, size, modular_decrement);
        }
    }

    static void mul(ThisType &res, const ThisType &a, const ThisType &b)
    {
        mp_limb_t mul_res[size * 2];
        mpn_mul_n(mul_res, a.data, b.data, size);

#ifndef NDEBUG
        static constexpr unsigned int mul_size = jw_util::FastMath::div_ceil<unsigned int>(modular_exponent * 2, GMP_NUMB_BITS);
        for (unsigned int i = mul_size; i < size * 2; i++)
        {
            assert(mul_res[i] == 0);
        }
#endif

        if (unused_bits != 0)
        {
            mp_limb_t carry = mpn_lshift(mul_res + size - 1, mul_res + size - 1, size + 1, unused_bits);
            assert(carry == 0);
        }

        mul_res[size - 1] >>= unused_bits;
        assert((mul_res[size - 1] & ~head_mask) == 0);
        assert((mul_res[size * 2 - 1] & ~head_mask) == 0);

        static constexpr unsigned int modular_decrement_bits = jw_util::FastMath::log2_constexpr(modular_decrement) + !jw_util::FastMath::is_pow2(modular_decrement);
        static constexpr unsigned int high_head_bits = head_bits + modular_decrement_bits;

        if (modular_decrement != 1)
        {
            mp_limb_t carry = mpn_mul_1(mul_res + size, mul_res + size, size, modular_decrement);

            if (high_head_bits > GMP_NUMB_BITS)
            {
                // Multiplication could have overflown
                mul_res[size] += (carry << unused_bits) * modular_decrement;
            }
            else
            {
                assert(carry == 0);
            }
        }
        else
        {
            assert(high_head_bits <= GMP_NUMB_BITS);
        }

        mp_limb_t carry = mpn_add_n(res.data, mul_res, mul_res + size, size);

        if (high_head_bits >= GMP_NUMB_BITS)
        {
            // Multiplication could have overflown
            carry <<= unused_bits;

            if (head_bits < GMP_NUMB_BITS)
            {
                carry |= res.data[size - 1] >> (head_bits % GMP_LIMB_BITS);
                res.data[size - 1] &= head_mask;
            }

            mpn_add_1(res.data, res.data, size, carry * modular_decrement);
        }
        else
        {
            assert(carry == 0);
            assert(head_bits < GMP_NUMB_BITS);

            carry = res.data[size - 1] >> (head_bits % GMP_LIMB_BITS);
            res.data[size - 1] &= head_mask;
            mpn_add_1(res.data, res.data, size, carry * modular_decrement);
        }

        assert((res.data[size - 1] & ~head_mask) == 0);
    }

    template <mp_limb_t fill>
    static void set_ambiguity(mp_limb_t *dst, mp_limb_t lsw)
    {
        dst[0] = lsw;
        if (size == 1)
        {
            dst[0] &= head_mask;
        }
        else
        {
            std::fill(dst + 1, dst + size - 1, fill);
            dst[size - 1] = fill & head_mask;
        }
    }

    static void set_mod(mp_limb_t *arr)
    {
        if (size == 1)
        {
            arr[0] = static_cast<mp_limb_t>(-static_cast<mp_limb_t>(modular_decrement)) & head_mask;
        }
        else
        {
            arr[0] = -static_cast<mp_limb_t>(modular_decrement);
            std::fill(arr + 1, arr + size - 1, GMP_NUMB_MASK);
            arr[size - 1] = head_mask;
        }
    }

    static void calc_inverse(ThisType &res, const ThisType &num)
    {
        mp_limb_t num_data[size];
        std::copy(num.data, num.data + size, num_data);

        mp_limb_t mod[size];
        set_mod(mod);

        assert(num != 0);
        assert(!std::equal(num.data, num.data + size, mod));

        mp_limb_t gcd[size];
        mp_limb_t x[size + 1];
        mp_size_t xn;
        mp_size_t limbs = mpn_gcdext(gcd, x, &xn, num_data, size, mod, size);

        assert(limbs == 1);
        assert(gcd[0] == 1);

        if (xn < 0)
        {
            std::fill(x - xn, x + size, 0);

            for (unsigned int i = 0; i < size - 1; i++)
            {
                assert((x[i] & ~GMP_NUMB_MASK) == 0);
                res.data[i] = x[i] ^ GMP_NUMB_MASK;
            }

            assert((x[size - 1] & ~head_mask) == 0);
            res.data[size - 1] = x[size - 1] ^ head_mask;

            res.data[0] -= modular_decrement - 1;
        }
        else
        {
            std::copy(x, x + xn, res.data);
            std::fill(res.data + xn, res.data + size, 0);
        }
    }
};

template <unsigned int modular_exponent, unsigned int modular_decrement>
IntModuloMersenne<modular_exponent, modular_decrement> &abs(IntModuloMersenne<modular_exponent, modular_decrement> &x) {return x;}

template <unsigned int modular_exponent, unsigned int modular_decrement>
const IntModuloMersenne<modular_exponent, modular_decrement> &abs(const IntModuloMersenne<modular_exponent, modular_decrement> &x) {return x;}

}


namespace Catch
{

template <unsigned int modular_exponent, unsigned modular_decrement>
std::string toString(const sonic_socket::IntModuloMersenne<modular_exponent, modular_decrement> &x)
{
    return x.to_string();
}

}

#endif // INTMODULOMERSENNE_H
