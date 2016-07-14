#ifndef FOUNTAINSOURCE_H
#define FOUNTAINSOURCE_H

#include <deque>
#include <algorithm>

// TODO: remove
#include <iostream>

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

    void generate_packet(Packet &packet)
    {
        // 16 bits: Max decoded remote symbol
        // 16 bits: First encoded symbol id
        // 16 bits: Num encoded symbols
        // 16 bits: Column element - Don't reuse this until the other endpoint has decoded all data points encoded by it.

        char *data_meta = packet.get_data();

        *data_meta++ = (static_cast<Derived*>(this)->get_decode_end() >> 0) & 0xFF;
        *data_meta++ = (static_cast<Derived*>(this)->get_decode_end() >> 8) & 0xFF;

        *data_meta++ = (encode_start >> 0) & 0xFF;
        *data_meta++ = (encode_start >> 8) & 0xFF;
        needs_encode_start_update = false;

        unsigned int encode_count = symbols.size();
        *data_meta++ = (encode_count >> 0) & 0xFF;
        *data_meta++ = (encode_count >> 8) & 0xFF;

        *data_meta++ = (cauchy_element >> 0) & 0xFF;
        *data_meta++ = (cauchy_element >> 8) & 0xFF;

        cauchy_element++;

        if (symbols.empty())
        {
            packet.set_size(packet_metadata_size);
        }
        else
        {
            SymbolType packet_symbols[symbols_per_packet];
            unsigned int num_packet_symbols = encode_count < symbols_per_packet ? encode_count : symbols_per_packet;

            std::deque<SymbolType>::const_iterator i = symbols.cbegin();
            unsigned int col = encode_start / symbols_per_packet;
            unsigned int matrix_id = encode_start % symbols_per_packet;
            SymbolType::BaseType *inv = SymbolType(cauchy_element + col).inverse();

            std::fill_n(packet_symbols + matrix_id, num_packet_symbols, SymbolType(0));

            while (true)
            {
                // The cool stuff happens here:
                packet_symbols[matrix_id] += (*i) * (*inv);

                //std::cout << "matrix " << matrix_id << " += " << (*i).to_string() << " * " << (*inv).to_string() << " -> " << packet_symbols[matrix_id].to_string() << std::endl;

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

            mp_limb_t *data_words = get_packet_words(packet);
            assert(data_meta == reinterpret_cast<const char *>(data_words));

            unsigned int data_size_symbols = jw_util::FastMath::div_ceil<unsigned int>(num_packet_symbols * SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT, GMP_LIMB_BITS);
            unsigned int data_size_chars = jw_util::FastMath::div_ceil<unsigned int>(num_packet_symbols * SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT, CHAR_BIT);

            std::fill_n(data_words, data_size_symbols, 0);

            //std::cout << "Send: ";

            unsigned int chunk = (encode_start + encode_count) % symbols_per_packet;
            unsigned int bit_offset = 0;
            for (unsigned int i = 0; i < num_packet_symbols; i++)
            {
                if (chunk == 0) {chunk = symbols_per_packet;}
                chunk--;

                //std::cout << "0x" << packet_symbols[chunk].to_string<16>() << " ";
                packet_symbols[chunk].write_to<true>(data_words, bit_offset);
            }
            //std::cout << std::endl;

            packet.set_size(packet_metadata_size + data_size_chars);
        }

        packet_mangle(packet);
    }

    bool has_data_to_send() const {return symbols.size() || needs_encode_start_update;}

protected:
    unsigned int encode_start = 0;
    unsigned int cauchy_element = 0;
    bool needs_encode_start_update = false;

private:
    std::deque<SymbolType> symbols;

    void update_encode_start(unsigned int new_encode_start)
    {
        unsigned int erase = new_encode_start - encode_start;

        if (erase)
        {
            encode_start = new_encode_start;
            symbols.erase(symbols.begin(), symbols.begin() + erase);

            needs_encode_start_update = true;
        }
    }
};

}

#endif // FOUNTAINSOURCE_H
