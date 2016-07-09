#ifndef SONICSOCKET_EIGEN_H
#define SONICSOCKET_EIGEN_H

#define EIGEN_DONT_PARALLELIZE
#define EIGEN_NO_AUTOMATIC_RESIZING

#include <eigen3/Eigen/Dense>

#include "libSonicSocket/intmodulomersenne.h"

namespace Eigen
{

template<unsigned int modular_exponent, unsigned int modular_decrement>
struct NumTraits<sonic_socket::IntModuloMersenne<modular_exponent, modular_decrement>>
{
    typedef sonic_socket::IntModuloMersenne<modular_exponent, modular_decrement> Real;
    typedef sonic_socket::IntModuloMersenne<modular_exponent, modular_decrement> Nested;

    enum {
        IsComplex = 0,
        IsInteger = 0,
        IsSigned = 0,
        RequireInitialization = 0,
        ReadCost = 1,
        AddCost = 2,
        MulCost = 3
    };
};

}

#endif // SONICSOCKET_EIGEN_H
