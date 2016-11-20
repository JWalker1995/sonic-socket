#ifndef MESSAGEAMBIGRESOLVER_H
#define MESSAGEAMBIGRESOLVER_H

#include <algorithm>

#include "libSonicSocket/config/SS_FOUNTAINCODER_SYMBOL_MODULAR_DECREMENT.h"

namespace sonic_socket
{

class MessageAmbigResolver {
public:
    static constexpr unsigned int ambig_bits = std::max(2, SS_FOUNTAINCODER_SYMBOL_MODULAR_DECREMENT);
};

}

#endif // MESSAGEAMBIGRESOLVER_H
