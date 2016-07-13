#ifndef FOUNTAINSINK_H
#define FOUNTAINSINK_H

#include <vector>
#include <deque>

// TODO: remove
#include <iostream>

#include "libSonicSocket/config/JWUTIL_CACHELRU_FORGET_POOL_ON_HEAP.h"
#include "libSonicSocket/jw_util/cachelru.h"

#include "libSonicSocket/config/SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT.h"

#include "libSonicSocket/fountainbase.h"
#include "libSonicSocket/logproxy.h"

namespace sonic_socket
{

template <typename Derived>
class FountainSource;

template <typename Derived>
class FountainSink : public FountainBase
{
    friend class FountainSource<Derived>;

public:
    class DecodedPacket
    {
        friend class FountainSink;

    public:
        bool has_symbol() const
        {
            return decode_i != decode_end;
        }

        SymbolType &get_symbol() const
        {
            assert(has_symbol());
            return *decode_i;
        }

        void next_symbol()
        {
            assert(has_symbol());
            decode_i++;
        }

    private:
        unsigned int num_packet_symbols;
        unsigned int prev_decode_end;

        std::deque<SymbolType>::iterator decode_i;
        std::deque<SymbolType>::iterator decode_end;
    };

    bool recv_packet(Packet &packet, DecodedPacket &decoded_packet)
    {
        // 16 bits: Max decoded remote symbol
        // 16 bits: First encoded symbol id
        // 16 bits: Num encoded symbols
        // 16 bits: Sequence element - Don't reuse this until the other endpoint has decoded all data points encoded by it.

        if (packet.get_size() < packet_metadata_size)
        {
            std::string error_msg = "Invalid packet size (" + std::to_string(packet.get_size()) + ", expected greater than " + std::to_string(packet_metadata_size) + ")";
            register_decode_error(error_msg);
            return false;
        }

        packet_demangle(packet);

        const char *data_meta = packet.get_data();

        unsigned int new_encode_start = 0;
        new_encode_start |= *data_meta++ >> 0;
        new_encode_start |= *data_meta++ >> 8;

        unsigned int symbol_start = 0;
        symbol_start |= *data_meta++ >> 0;
        symbol_start |= *data_meta++ >> 8;

        unsigned int symbol_count = 0;
        symbol_count |= *data_meta++ >> 0;
        symbol_count |= *data_meta++ >> 8;

        unsigned int cauchy_element = 0;
        cauchy_element |= *data_meta++ >> 0;
        cauchy_element |= *data_meta++ >> 8;
        cauchy_element++;

        static_cast<Derived*>(this)->update_encode_start(new_encode_start);

        unsigned int symbol_end = symbol_start + symbol_count;

        unsigned int num_packet_symbols = symbol_count < symbols_per_packet ? symbol_count : symbols_per_packet;

        unsigned int expected_size = packet_metadata_size + jw_util::FastMath::div_ceil<unsigned int>(num_packet_symbols * SS_FOUNTAININTERFACE_SYMBOL_MODULAR_EXPONENT, CHAR_BIT);
        if (packet.get_size() != expected_size)
        {
            std::string error_msg = "Invalid packet size (";
                error_msg += "packet.get_size()=" + std::to_string(packet.get_size());
                error_msg += ", expected_size=" + std::to_string(expected_size);
            error_msg += ")";
            register_decode_error(error_msg);
            return false;
        }

        if (symbol_start < decoded_offset)
        {
            std::string error_msg = "Packet includes forgotten symbols (";
                error_msg += "symbol_start=" + std::to_string(symbol_start);
                error_msg += ", decoded_offset=" + std::to_string(decoded_offset);
            error_msg += ")";
            register_decode_error(error_msg);
            return false;
        }

        unsigned int decode_end = get_decode_end();
        if (symbol_start > decode_end)
        {
            std::string error_msg = "Packet doesn't include some un-decoded symbols (";
                error_msg += "symbol_start=" + std::to_string(symbol_start);
                error_msg += ", decode_end=" + std::to_string(decode_end);
            error_msg += ")";
            register_decode_error(error_msg);
            return false;
        }

        const mp_limb_t *data_words = get_packet_words(packet);
        assert(data_meta == reinterpret_cast<const char *>(data_words));
        unsigned int bit_offset = 0;

        unsigned int matrix_split_1 = symbol_start % symbols_per_packet;
        unsigned int matrix_split_2 = symbol_end % symbols_per_packet;
        unsigned int col_start = symbol_start / symbols_per_packet;
        unsigned int col_count = symbol_count / symbols_per_packet;

        std::cout << "Recv: ";

        for (unsigned int i = symbol_end; i-- > symbol_start; )
        {
            bool col_start_inc = i < matrix_split_1;

            // If matrix_split_1 < matrix_split_2: 0 [ms1] 1 [ms2] 0
            // If matrix_split_1 > matrix_split_2: 1 [ms1] 0 [ms2] 1
            bool col_count_inc = (matrix_split_1 <= i) ^ (i < matrix_split_2) ^ (matrix_split_1 < matrix_split_2);

            MatrixGenerator &cur_mat = chunks[i];

            typename MatrixGenerator::Row row;
            row.cauchy_element = cauchy_element;
            row.col_start = col_start + col_start_inc;
            row.col_end = row.col_start + col_count + col_count_inc;
            row.sum.template read_from<true>(data_words, bit_offset);

            std::cout << "0x" << row.sum.template to_string<16>() << " ";

            unsigned int subtract_id = row.col_start * symbols_per_packet + i;
            while (subtract_id < decode_end)
            {
                assert(subtract_id >= symbol_start);
                assert(subtract_id < symbol_end);

                assert(subtract_id >= decoded_offset);
                assert(subtract_id < decode_end);

                SymbolType::BaseType *inverse = SymbolType(row.cauchy_element + row.col_start).inverse();
                row.sum -= decoded[subtract_id - decoded_offset] * (*inverse);

                row.col_start++;
                subtract_id += symbols_per_packet;
            }

            if (row.col_start == row.col_end)
            {
                if (row.sum != 0)
                {
                    revert_packet(i + 1, num_packet_symbols, decode_end);

                    std::string error_msg = "Packet symbols contradict previous packets (";
                        error_msg += "i=" + std::to_string(i);
                        error_msg += ", row.cauchy_element=" + std::to_string(row.cauchy_element);
                        error_msg += ", row.col_start=" + std::to_string(row.col_start);
                        error_msg += ", row.col_end=" + std::to_string(row.col_end);
                        error_msg += ", row.sum=" + row.sum.to_string();
                    error_msg += ")";
                    register_decode_error("Packet symbols contradict previous packets");
                    return false;
                }
            }
            else
            {
                cur_mat.add_row(row);

                unsigned int cur_mat_cols = cur_mat.max_col_end - cur_mat.min_col_start;
                assert(cur_mat_cols >= cur_mat.rows.size());
                if (cur_mat_cols == cur_mat.rows.size())
                {
                    decode_matrix(i, cur_mat);
                    cur_mat.reset_rows();
                }
            }
        }
        std::cout << std::endl;

        use_symbols(symbol_start);

        register_decode_success();

        decoded_packet.num_packet_symbols = num_packet_symbols;
        decoded_packet.prev_decode_end = decode_end;
        decoded_packet.decode_i = decoded.begin() + decode_end;
        decoded_packet.decode_end = decoded.end();
        return true;
    }

    bool has_symbol() const
    {
        return decoded_read_pos < decoded.size();
    }

    const SymbolType &get_symbol() const
    {
        assert(has_symbol());
        return decoded[decoded_read_pos];
    }

    void next_symbol()
    {
        assert(has_symbol());
        decoded_read_pos++;
    }

    void report_decode_error(const DecodedPacket &decoded_packet, const std::string &error_msg)
    {
        revert_packet(0, decoded_packet.num_packet_symbols, decoded_packet.prev_decode_end);
        register_decode_error(error_msg);
    }

private:
    // Each source message is a column
    // Each generated packet is a row

    struct MatrixGenerator
    {
        struct Row
        {
            unsigned int cauchy_element;
            unsigned int col_start;
            unsigned int col_end;
            SymbolType sum;

            bool operator==(const Row &other) const
            {
                return cauchy_element == other.cauchy_element
                    && col_start == other.col_start
                    && col_end == other.col_end;
            }
        };
        std::vector<Row> rows;

        unsigned int min_col_start = static_cast<unsigned int>(-1);
        unsigned int max_col_end = 0;

        std::size_t rows_hash = 0;

        void reset_rows()
        {
            rows.clear();
            min_col_start = static_cast<unsigned int>(-1);
            max_col_end = 0;
            rows_hash = 0;
        }

        void add_row(const Row &row)
        {
            rows.push_back(row);

            if (row.col_start < min_col_start) {min_col_start = row.col_start;}
            if (row.col_end > max_col_end) {max_col_end = row.col_end;}

            rows_hash = jw_util::Hash::combine(rows_hash, row.cauchy_element);
            rows_hash = jw_util::Hash::combine(rows_hash, row.col_start);
            rows_hash = jw_util::Hash::combine(rows_hash, row.col_end);
        }

        bool operator==(const MatrixGenerator &other) const
        {
            // Only compares matrix, not right-hand side (Row::sum)

            assert(rows_hash == other.rows_hash);
            return rows == other.rows;
        }

        struct Hasher
        {
            std::size_t operator()(const MatrixGenerator &obj) const
            {
                return obj.rows_hash;
            }
        };
    };

    // TODO:
    // unsigned int inverse_cache_offset = 1;
    // std::deque<SymbolType> inverse_cache;

    MatrixGenerator chunks[symbols_per_packet];

    unsigned int decoded_offset = 0;
    unsigned int decoded_read_pos = 0;
    std::deque<SymbolType> decoded;

    unsigned int decoded_use_begin = 0;
    unsigned int decoded_use_count = 0;

    typedef jw_util::CacheLRU<MatrixGenerator, SymbolMatrixType, 1024, typename MatrixGenerator::Hasher> MatrixInverseCacheType;
    MatrixInverseCacheType matrix_inverse_cache;

    float error_accumulator = 0.0f;

    unsigned int get_decode_end() const
    {
        return decoded_offset + decoded.size();
    }

    void decode_matrix(unsigned int chunk, MatrixGenerator &generator)
    {
        assert(&chunks[chunk] == &generator);

        typename MatrixInverseCacheType::Result inverse = matrix_inverse_cache.access(generator);

        if (!inverse.is_valid())
        {
            calc_matrix_inverse(*inverse.get_value(), generator);
        }

        unsigned int size = generator.rows.size();
        unsigned int decoded_resize_to = (generator.max_col_end - 1) * symbols_per_packet + chunk - decoded_offset;
        if (decoded.size() <= decoded_resize_to)
        {
            decoded.resize(decoded_resize_to + 1);
        }

        unsigned int decoded_i = decoded_resize_to - size * symbols_per_packet;

        SymbolType *data = inverse.get_value()->data();
        for (unsigned int i = 0; i < size; i++)
        {
            SymbolType sum(0);

            typename std::vector<typename MatrixGenerator::Row>::const_iterator j = generator.rows.cbegin();
            while (j != generator.rows.cend())
            {
                sum += (*data) * j->sum;
                data++;
                j++;
            }

            decoded_i += symbols_per_packet;
            decoded[decoded_i] = sum;
        }

        assert(decoded_i == decoded_resize_to);
    }

    void calc_matrix_inverse(SymbolMatrixType &res, const MatrixGenerator &generator) const
    {
        SymbolMatrixType mat;
        generate_matrix(mat, generator);

        res = mat.lu().inverse();
    }

    void generate_matrix(SymbolMatrixType &res, const MatrixGenerator &generator) const
    {
        unsigned int size = generator.rows.size();
        res.resize(size, size);

        static_assert(SymbolMatrixType::IsRowMajor, "SymbolMatrixType must be stored row-major");
        SymbolType *i = res.data();

        typename std::vector<typename MatrixGenerator::Row>::const_iterator j = generator.rows.cbegin();
        while (j != generator.rows.cend())
        {
            unsigned int k = generator.min_col_start;
            while (k < j->col_start)
            {
                *i = SymbolType(0);
                i++;
                k++;
            }

            while (k < j->col_end)
            {
                *i = *SymbolType(j->cauchy_element + k).inverse();
                i++;
                k++;
            }

            while (k < generator.max_col_end)
            {
                *i = SymbolType(0);
                i++;
                k++;
            }

            j++;
        }

        assert(i == res.data() + size * size);
    }

    void revert_packet(unsigned int chunk_begin, unsigned int chunk_end, unsigned int prev_decode_end)
    {
        for (unsigned int i = chunk_begin; i != chunk_end; i++)
        {
            MatrixGenerator &chunk = chunks[i];
            if (chunk.rows.empty())
            {
                // Just decoded this matrix
            }
            else
            {
                // Just added a row to this matrix, so remove it
                chunk.rows.pop_back();

                // The min_col_start/max_col_start and rows_hash are going to be incorrect,
                //   so reset them and re-insert all rows.
                // This isn't the fastest way to do this, but hopefully this function won't be called that often.
                std::vector<typename MatrixGenerator::Row> rows = std::move(chunk.rows);
                chunk.reset_rows();

                typename std::vector<typename MatrixGenerator::Row>::const_iterator j = rows.cbegin();
                while (j != rows.cend())
                {
                    chunk.add_row(*j);
                    j++;
                }
            }
        }

        // Remove decoded messages
        assert(prev_decode_end >= decoded_offset);
        decoded.resize(prev_decode_end - decoded_offset);
    }

    void use_symbols(unsigned int begin)
    {
        if (begin < decoded_use_begin) {decoded_use_begin = begin;}

        decoded_use_count++;
        if (decoded_use_count == 4)
        {
            assert(decoded_use_begin <= decoded_read_pos);
            assert(decoded_use_begin <= decoded.size());

            decoded_offset += decoded_use_begin;
            decoded_read_pos -= decoded_use_begin;
            decoded.erase(decoded.begin(), decoded.begin() + decoded_use_begin);

            decoded_use_begin = decoded_read_pos;
            decoded_use_count = 0;
        }
    }

    void register_decode_error(const std::string &error_msg)
    {
        LogProxy &logger = static_cast<Derived *>(this)->get_logger();

        logger.push_log(LogProxy::LogLevel::Warning, error_msg);

        error_accumulator += 1.0f;
        if (error_accumulator > 4.0f)
        {
            logger.push_log(LogProxy::LogLevel::Fatal, "Seeing an awful lot of network errors...");
        }
    }

    void register_decode_success()
    {
        error_accumulator *= 0.8f;
    }
};

}

#endif // FOUNTAINSINK_H
