#ifndef FOUNTAINBASE_H
#define FOUNTAINBASE_H

#include "libSonicSocket/config/SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT.h"
#include "libSonicSocket/config/SS_FOUNTAININTERFACE_SYMBOL_MODULAR_DECREMENT.h"
#include "libSonicSocket/config/SS_MAX_PACKET_SIZE.h"
#include "libSonicSocket/config/SS_MANGLE_PASSES.h"

#include "libSonicSocket/eigen.h"
#include "libSonicSocket/intmodulomersennestorage.h"
#include "libSonicSocket/packetmangler.h"
#include "libSonicSocket/logproxy.h"

namespace sonic_socket
{

class FountainBase
{
public:
    typedef IntModuloMersenneStorage<SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT, SS_FOUNTAININTERFACE_SYMBOL_MODULAR_DECREMENT> SymbolType;

protected:
    typedef Eigen::Matrix<SymbolType, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> SymbolMatrixType;
    typedef Eigen::Matrix<SymbolType, Eigen::Dynamic, 1> SymbolVectorType;

    static constexpr unsigned int packet_metadata_size = 8;
    static constexpr unsigned int symbols_per_packet = (SS_MAX_PACKET_SIZE - packet_metadata_size) * CHAR_BIT / SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT;

public:
    class Packet
    {
        friend class FountainBase;

    public:
        Packet()
        {
            if (symbol_start_word * sizeof(mp_limb_t) != packet_metadata_size)
            {
                words[0] = 0;
            }
        }

        unsigned char *get_data()
        {
            return reinterpret_cast<unsigned char *>(words) + data_offset;
        }

        unsigned int get_size() const {return size;}
        unsigned int &get_mutable_size() {return size;}
        void set_size(unsigned int new_size) {size = new_size;}

    private:
        static constexpr unsigned int symbol_start_word = jw_util::FastMath::div_ceil<unsigned int>(packet_metadata_size, sizeof(mp_limb_t));
        static constexpr unsigned int data_offset = symbol_start_word * sizeof(mp_limb_t) - packet_metadata_size;

        mp_limb_t words[jw_util::FastMath::div_ceil(SS_MAX_PACKET_SIZE * CHAR_BIT, GMP_LIMB_BITS) + 2];

        unsigned int size;
    };

    static void static_init(LogProxy &logger)
    {
        if (!jw_util::FastMath::is_pow2(symbols_per_packet))
        {
            logger.push_event<LogProxy::LogLevel::Debug>("FountainBase::symbols_per_packet = ", std::to_string(symbols_per_packet), " is not a power of two");
        }
    }

protected:
    static mp_limb_t *get_packet_words(Packet &packet)
    {
        return packet.words + Packet::symbol_start_word;
    }

    static void packet_mangle(Packet &packet)
    {
        PacketMangler<mp_limb_t>::mangle<SS_MANGLE_PASSES>(packet.words, Packet::data_offset, Packet::data_offset + packet.get_size());
    }

    static void packet_demangle(Packet &packet)
    {
        PacketMangler<mp_limb_t>::demangle<SS_MANGLE_PASSES>(packet.words, Packet::data_offset, Packet::data_offset + packet.get_size());
    }
};

}

#endif // FOUNTAINBASE_H
