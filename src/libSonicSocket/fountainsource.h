#ifndef FOUNTAINSOURCE_H
#define FOUNTAINSOURCE_H

#include <deque>
#include <algorithm>

#include "libSonicSocket/config/SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT.h"
#include "libSonicSocket/config/SS_FOUNTAININTERFACE_SYMBOL_MODULAR_DECREMENT.h"
#include "libSonicSocket/config/SS_MAX_PACKET_SIZE.h"
#include "libSonicSocket/config/SS_MANGLE_PASSES.h"

#include "libSonicSocket/fountainbase.h"

namespace sonic_socket
{

template <typename Derived>
class FountainSink;

template <typename Derived>
class FountainSource : public FountainBase
{
    friend class FountainSink<Derived>;

public:
    SymbolType &alloc_symbol()
    {
        symbols.emplace_back();
        return symbols.back();
    }

    void generate_init_packet(Packet &packet)
    {
        const char *magic = "SonicSocket!";
        static constexpr unsigned int magic_length = 13;

        mp_limb_t tmp[jw_util::FastMath::div_ceil<unsigned int>(magic_length, sizeof(mp_limb_t))];

        char *data = packet.get_data();

        // Write magic phrase
        std::copy_n(magic, magic_length, data);
        data += magic_length;

        // Write packet metadata size
        *data++ = (packet_metadata_size >> 0) & 0xFF;
        *data++ = (packet_metadata_size >> 8) & 0xFF;

        // Write symbols per packet
        *data++ = (symbols_per_packet >> 0) & 0xFF;
        *data++ = (symbols_per_packet >> 8) & 0xFF;

        // Write max packet size
        *data++ = (SS_MAX_PACKET_SIZE >> 0) & 0xFF;
        *data++ = (SS_MAX_PACKET_SIZE >> 8) & 0xFF;

        // Write mangle passes
        *data++ = (SS_MANGLE_PASSES >> 0) & 0xFF;
        *data++ = (SS_MANGLE_PASSES >> 8) & 0xFF;

        // Write symbol modular exponent
        *data++ = (SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT >> 0) & 0xFF;
        *data++ = (SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT >> 8) & 0xFF;

        // Write symbol modular decrement
        *data++ = (SS_FOUNTAININTERFACE_SYMBOL_MODULAR_DECREMENT >> 0) & 0xFF;
        *data++ = (SS_FOUNTAININTERFACE_SYMBOL_MODULAR_DECREMENT >> 8) & 0xFF;

        // Write mangled data sample
        std::copy_n(magic, magic_length, reinterpret_cast<char *>(tmp));
        PacketMangler<mp_limb_t>::mangle<SS_MANGLE_PASSES>(tmp, 0, magic_length);
        std::copy_n(reinterpret_cast<char *>(tmp), magic_length, data);
        data += magic_length;

        // Write demangled data sample
        std::copy_n(magic, magic_length, reinterpret_cast<char *>(tmp));
        PacketMangler<mp_limb_t>::demangle<SS_MANGLE_PASSES>(tmp, 0, magic_length);
        std::copy_n(reinterpret_cast<char *>(tmp), magic_length, data);
        data += magic_length;

        packet.set_size(data - packet.get_data());
    }

    bool generate_packet(Packet &packet)
    {
        // 16 bits: Max decoded remote symbol
        // 16 bits: First encoded symbol id
        // 16 bits: Num encoded symbols
        // 16 bits: Column element - Don't reuse this until the other endpoint has decoded all data points encoded by it.

        if (symbols.empty()) {return false;}

        char *data_meta = packet.get_data();

        *data_meta++ = (static_cast<Derived*>(this)->get_decode_end() >> 0) & 0xFF;
        *data_meta++ = (static_cast<Derived*>(this)->get_decode_end() >> 8) & 0xFF;

        *data_meta++ = (encode_start >> 0) & 0xFF;
        *data_meta++ = (encode_start >> 8) & 0xFF;

        *data_meta++ = (symbols.size() >> 0) & 0xFF;
        *data_meta++ = (symbols.size() >> 8) & 0xFF;

        *data_meta++ = (cauchy_element >> 0) & 0xFF;
        *data_meta++ = (cauchy_element >> 8) & 0xFF;

        cauchy_element++;

        SymbolType packet_symbols[symbols_per_packet];
        unsigned int num_packet_symbols = symbols.size() < symbols_per_packet ? symbols.size() : symbols_per_packet;
        std::fill_n(packet_symbols, num_packet_symbols, SymbolType(0));

        std::deque<SymbolType>::const_iterator i = symbols.cbegin();
        unsigned int col = encode_start / symbols_per_packet;
        unsigned int matrix_id = encode_start % symbols_per_packet;
        SymbolType::BaseType *inv = SymbolType(cauchy_element + col).inverse();

        while (true)
        {
            // The cool stuff happens here:
            packet_symbols[matrix_id] += (*i) * (*inv);

            i++;
            if (i == symbols.cend()) {break;}

            matrix_id++;
            if (matrix_id == symbols_per_packet)
            {
                matrix_id = 0;
                col++;
                inv = SymbolType(cauchy_element + col).inverse();
            }
        }

        mp_limb_t *data_symbol = get_packet_words(packet);
        assert(data_meta == reinterpret_cast<const char *>(data_symbol));

        unsigned int data_size_symbols = jw_util::FastMath::div_ceil<unsigned int>(num_packet_symbols * SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT, GMP_LIMB_BITS);
        unsigned int data_size_chars = jw_util::FastMath::div_ceil<unsigned int>(num_packet_symbols * SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT, CHAR_BIT);
        
        std::fill_n(data_symbol, data_size_symbols, 0);

        unsigned int bit_offset = 0;
        for (unsigned int i = num_packet_symbols; i-- > 0; )
        {
            packet_symbols[i].write_to<true>(data_symbol, bit_offset);
        }

        packet.set_size(packet_metadata_size + data_size_chars);

        packet_mangle(packet);

        return true;
    }

protected:
    unsigned int encode_start = 0;
    unsigned int cauchy_element = 0;

private:
    std::deque<SymbolType> symbols;

    void update_encode_start(unsigned int new_encode_start)
    {
        unsigned int erase = new_encode_start - encode_start;
        encode_start = new_encode_start;

        symbols.erase(symbols.begin(), symbols.begin() + erase);
    }
};

}

#endif // FOUNTAINSOURCE_H
