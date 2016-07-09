#ifndef CAUCHYMATRIX_H
#define CAUCHYMATRIX_H

#include <type_traits>
#include <assert.h>

namespace sonic_socket
{

template <typename SeqElType, typename MatrixElType>
class CauchyMatrix
{
public:
    CauchyMatrix(unsigned int size, const SeqElType *x, const SeqElType *y)
        : size(size)
        , x(x)
        , y(y)
    {}

    void calc_inverse(MatrixElType *res)
    {
        static_assert(std::type_traits<SeqElType>::is_signed, "CauchyMatrix<SeqElType, MatrixElType>::calc_inverse : SeqElType must be signed");

        static std::vector<MatrixElType> cache;
        cache.resize(size * 4);

        for (unsigned int i = 0; i < size; i++)
        {
            MatrixElType top1 = x[i] + y[0];
            MatrixElType bot1 = i ? x[i] - x[0] : static_cast<SeqElType>(1);
            MatrixElType top2 = x[0] + y[i];
            MatrixElType bot2 = i ? y[i] - y[0] : static_cast<SeqElType>(1);

            for (unsigned int j = 1; j < size; j++)
            {
                top1 *= x[i] + y[j];
                top2 *= x[j] + y[i];
                if (i != j)
                {
                    bot1 *= x[i] - x[j];
                    bot2 *= y[i] - y[j];
                }
            }

#ifndef NDEBUG
            assert(top1 != 0);
            assert(top2 != 0);
            assert(bot1 != 0);
            assert(bot2 != 0);
#endif

            cache[i * 4 + 0] = top1;
            cache[i * 4 + 1] = bot1;
            cache[i * 4 + 2] = top2;
            cache[i * 4 + 3] = bot2;
        }

        for (unsigned int i = 0; i < size; i++)
        {
            MatrixElType top1 = cache[i * 4 + 0];
            MatrixElType bot1 = cache[i * 4 + 1];

            for (unsigned int j = 0; j < size; j++)
            {
                MatrixElType top2 = cache[j * 4 + 2];
                MatrixElType bot2 = cache[j * 4 + 3];

                res[i * size + j] = (top1 * top2) / ((x[i] + y[j]) * bot1 * bot2);
            }
        }
    }

    void calc_matrix(MatrixElType *res)
    {
        for (unsigned int i = 0; i < size; i++)
        {
            for (unsigned int j = 0; j < size; j++)
            {
                res[i * size + j] = static_cast<MatrixElType>(1) / (x[i] + y[j]);
            }
        }
    }

private:
    unsigned int size;
    const SeqElType *x;
    const SeqElType *y;
};

}

#endif // CAUCHYMATRIX_H
