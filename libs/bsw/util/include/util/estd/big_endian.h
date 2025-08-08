// Copyright 2021 Accenture.

#ifndef GUARD_F2437631_4008_4301_B9A3_62725EA4B459
#define GUARD_F2437631_4008_4301_B9A3_62725EA4B459

#include <etl/algorithm.h>
#include <util/estd/assert.h>

#include <platform/estdint.h>

#include <limits>

/* suppress misra 5.0.15 EOF: The entire purpose of this file is to apply the subscript operator to
 * a pointer value to use array indexing to transform the data */
/* suppress autosar A5.3.2 EOF: The entire purpose of this file is to apply the subscript operator
 * to a pointer value to use array indexing to transform the data */

// suppress misra 14.5.1 EOF: Generic functions allowed in namespace estd by design
// suppress misra 3.4.1 EOF: types are declared here for reuse

namespace estd
{
namespace internal
{
// suppress misra 2.10.5 next_construct: (FP) no static storage version conflicting
template<typename T, uint32_t N>
T read_be(uint8_t const*);

// suppress misra 2.10.5 next_construct: (FP) no static storage version conflicting
template<typename T, uint32_t N>
void write_be(uint8_t*, T);

// clang-format off
#if defined(ESR_COMPILER_DIAB) && defined(__PPC_VLE__) && (__BIG_ENDIAN__ == 1) && (!defined(ESR_DISABLE_UNALIGNED_RW))
// clang-format on

// We're forcing this optimization on diab (it doesn't do it by itself).
// Here we assume that unaligned read/write is allowed - which holds true for all known targets.

template<>
inline uint16_t read_be<uint16_t, 2>(uint8_t const* const s)
{
    return *(reinterpret_cast<uint16_t const*>(s));
}

template<>
inline uint32_t read_be<uint32_t, 4>(uint8_t const* const s)
{
    return *(reinterpret_cast<uint32_t const*>(s));
}

template<>
inline uint64_t read_be<uint64_t, 8>(uint8_t const* const s)
{
    return *(reinterpret_cast<uint64_t const*>(s));
}

template<>
inline void write_be<uint16_t, 2>(uint8_t* const s, uint16_t const value)
{
    *(reinterpret_cast<uint16_t*>(s)) = value;
}

template<>
inline void write_be<uint32_t, 4>(uint8_t* const s, uint32_t const value)
{
    *(reinterpret_cast<uint32_t*>(s)) = value;
}

template<>
inline void write_be<uint64_t, 8>(uint8_t* const s, uint64_t const value)
{
    *(reinterpret_cast<uint64_t*>(s)) = value;
}

#else

template<>
inline uint16_t read_be<uint16_t, 2>(uint8_t const* const s)
{
    return static_cast<uint16_t>(static_cast<uint16_t>(s[0]) << 8U) | static_cast<uint16_t>(s[1]);
}

template<>
inline uint32_t read_be<uint32_t, 4>(uint8_t const* const s)
{
    return (static_cast<uint32_t>(s[0]) << 24U) | (static_cast<uint32_t>(s[1]) << 16U)
           | (static_cast<uint32_t>(s[2]) << 8U) | static_cast<uint32_t>(s[3]);
}

template<>
inline uint64_t read_be<uint64_t, 8>(uint8_t const* const s)
{
    return (static_cast<uint64_t>(s[0]) << 56U) | (static_cast<uint64_t>(s[1]) << 48U)
           | (static_cast<uint64_t>(s[2]) << 40U) | (static_cast<uint64_t>(s[3]) << 32U)
           | (static_cast<uint64_t>(s[4]) << 24U) | (static_cast<uint64_t>(s[5]) << 16U)
           | (static_cast<uint64_t>(s[6]) << 8U) | static_cast<uint64_t>(s[7]);
}

template<>
inline void write_be<uint16_t, 2>(uint8_t* const s, uint16_t const value)
{
    s[0] = static_cast<uint8_t>(value >> 8U);
    s[1] = static_cast<uint8_t>(value);
}

template<>
inline void write_be<uint32_t, 4>(uint8_t* const s, uint32_t const value)
{
    s[0] = static_cast<uint8_t>(value >> 24U);
    s[1] = static_cast<uint8_t>(value >> 16U);
    s[2] = static_cast<uint8_t>(value >> 8U);
    s[3] = static_cast<uint8_t>(value);
}

template<>
inline void write_be<uint64_t, 8>(uint8_t* const s, uint64_t const value)
{
    s[0] = static_cast<uint8_t>(value >> 56U);
    s[1] = static_cast<uint8_t>(value >> 48U);
    s[2] = static_cast<uint8_t>(value >> 40U);
    s[3] = static_cast<uint8_t>(value >> 32U);
    s[4] = static_cast<uint8_t>(value >> 24U);
    s[5] = static_cast<uint8_t>(value >> 16U);
    s[6] = static_cast<uint8_t>(value >> 8U);
    s[7] = static_cast<uint8_t>(value);
}

#endif

template<>
inline uint8_t read_be<uint8_t, 1>(uint8_t const* const s)
{
    return s[0];
}

template<>
inline uint32_t read_be<uint32_t, 3>(uint8_t const* const s)
{
    return (static_cast<uint32_t>(s[0]) << 16U) | (static_cast<uint32_t>(s[1]) << 8U)
           | static_cast<uint32_t>(s[2]);
}

template<>
inline uint64_t read_be<uint64_t, 6>(uint8_t const* const s)
{
    return (static_cast<uint64_t>(s[0]) << 40U) | (static_cast<uint64_t>(s[1]) << 32U)
           | (static_cast<uint64_t>(s[2]) << 24U) | (static_cast<uint64_t>(s[3]) << 16U)
           | (static_cast<uint64_t>(s[4]) << 8U) | static_cast<uint64_t>(s[5]);
}

template<>
inline int8_t read_be<int8_t, 1>(uint8_t const* const s)
{
    return static_cast<int8_t>(read_be<uint8_t, 1>(s));
}

template<>
inline int16_t read_be<int16_t, 2>(uint8_t const* const s)
{
    return static_cast<int16_t>(read_be<uint16_t, 2>(s));
}

template<>
inline int32_t read_be<int32_t, 4>(uint8_t const* const s)
{
    return static_cast<int32_t>(read_be<uint32_t, 4>(s));
}

template<>
inline int64_t read_be<int64_t, 8>(uint8_t const* const s)
{
    return static_cast<int64_t>(read_be<uint64_t, 8>(s));
}

template<>
inline void write_be<uint8_t, 1>(uint8_t* const s, uint8_t const value)
{
    s[0] = value;
}

template<>
inline void write_be<uint32_t, 3>(uint8_t* const s, uint32_t const value)
{
    s[0] = static_cast<uint8_t>(value >> 16U);
    s[1] = static_cast<uint8_t>(value >> 8U);
    s[2] = static_cast<uint8_t>(value >> 0U);
}

template<>
inline void write_be<uint64_t, 6>(uint8_t* const s, uint64_t const value)
{
    s[0] = static_cast<uint8_t>(value >> 40U);
    s[1] = static_cast<uint8_t>(value >> 32U);
    s[2] = static_cast<uint8_t>(value >> 24U);
    s[3] = static_cast<uint8_t>(value >> 16U);
    s[4] = static_cast<uint8_t>(value >> 8U);
    s[5] = static_cast<uint8_t>(value >> 0U);
}

template<>
inline void write_be<int8_t, 1>(uint8_t* const s, int8_t const value)
{
    write_be<uint8_t, 1>(s, static_cast<uint8_t>(value));
}

template<>
inline void write_be<int16_t, 2>(uint8_t* const s, int16_t const value)
{
    write_be<uint16_t, 2>(s, static_cast<uint16_t>(value));
}

template<>
inline void write_be<int32_t, 4>(uint8_t* const s, int32_t const value)
{
    write_be<uint32_t, 4>(s, static_cast<uint32_t>(value));
}

template<>
inline void write_be<int64_t, 8>(uint8_t* const s, int64_t const value)
{
    write_be<uint64_t, 8>(s, static_cast<uint64_t>(value));
}

} // namespace internal

template<typename T>
T read_be(uint8_t const* const s)
{
    return internal::read_be<T, sizeof(T)>(s);
}

template<typename T>
void write_be(uint8_t* const s, T const value)
{
    internal::write_be<T, sizeof(T)>(s, value);
}

namespace internal
{} // namespace internal

template<typename T, uint32_t N = sizeof(T)>
struct big_endian
{
    static_assert(N <= sizeof(T), "");

    uint8_t bytes[N];

    big_endian& operator=(T const value)
    {
        internal::write_be<T, N>(&bytes[0], value);
        return *this;
    }

    operator T() const { return internal::read_be<T, N>(&bytes[0]); }

    static big_endian make(T const value)
    {
        big_endian ret;
        ret = value;
        return ret;
    }
};

using be_uint16_t = big_endian<uint16_t>;
using be_uint32_t = big_endian<uint32_t>;
using be_uint64_t = big_endian<uint64_t>;

using be_uint24_t = big_endian<uint32_t, 3>;
using be_uint48_t = big_endian<uint64_t, 6>;

using be_int16_t = big_endian<int16_t>;
using be_int32_t = big_endian<int32_t>;
using be_int64_t = big_endian<int64_t>;

inline uint32_t read_be_24(uint8_t const* const s) { return internal::read_be<uint32_t, 3>(s); }

inline void write_be_24(uint8_t* const s, uint32_t const value)
{
    internal::write_be<uint32_t, 3>(s, value);
}

inline uint64_t read_be_48(uint8_t const* const s) { return internal::read_be<uint64_t, 6>(s); }

inline void write_be_48(uint8_t* const s, uint64_t const value)
{
    internal::write_be<uint64_t, 6>(s, value);
}

/**
 * @brief
 * Read BE bits from "left to right" (lowest index read first), and shift read result left with each
 * further read
 * \tparam T Returning integral type
 */
template<typename T>
T read_be_bits(uint8_t const* const src, size_t offset, size_t length)
{
    estd_assert(length <= sizeof(T) * 8);
    T result                  = 0;
    // Adjust offset to modulo of 8 for ease of calculation, and set index accordingly
    size_t idx                = offset / 8;
    offset                    = offset % 8;
    // Shift value for mask: as it's potentially only part of the first byte, min between length and
    // Number of bytes in first byte to read
    uint8_t const firstLength = static_cast<uint8_t>(::etl::min<size_t>(length, 8U - offset));
    // Shift value for return value: 8 - (offset + length), or 0 if offset + length >= 8
    uint8_t const firstShift  = 8U - static_cast<uint8_t>(::etl::min<size_t>(8U, offset + length));
    /* suppress misra 5.0.10 next_construct: The operations are on unsigned types and thus the
     * implicit conversions cannot be avoided due to signed bit arithmetic
     */
    // Take uint8_t max (all 1s), shift by number of bits to read, then invert (all 0s for length 0)
    uint8_t const firstMask   = ~(::std::numeric_limits<uint8_t>::max() << firstLength);

    // suppress misra 5.0.8 next_construct: Explicit conversions to bigger sizes are intentional
    // Set result to first byte, shifted & masked
    result = static_cast<T>((src[idx] >> firstShift) & firstMask);
    length -= firstLength;
    ++idx;

    while (length != 0)
    {
        if (length >= 8)
        {
            // Full byte copy, no shifting or masking needed
            if (sizeof(T) > 1)
            {
                result <<= 8;
            }
            result |= static_cast<T>(src[idx]);
            ++idx;
            length -= 8;
        }
        else
        {
            // Final partial byte read
            uint8_t const lastShift = static_cast<uint8_t>(8 - length);
            uint8_t const lastMask  = std::numeric_limits<uint8_t>::max() >> lastShift;
            result <<= length;
            // suppress misra 5.0.8 next_construct: Explicit conversions to bigger sizes are
            // intentional
            result |= static_cast<T>((src[idx] >> lastShift) & lastMask);
            ++idx;
            // Consume the rest of the length here. Same as length -= length
            length = 0;
        }
    }
    return result;
}

/**
 * @brief
 * Write BE bits from "right to left" (highest index written first), and shift input source right
 * with each further write
 * \tparam T Source integral type
 */
template<typename T>
void write_be_bits(uint8_t* const dst, T src, size_t offset, size_t length)
{
    estd_assert(length <= sizeof(T) * 8);
    // Adjust offset to modulo of 8 for ease of calculation, and set index accordingly
    size_t idx = (offset + length) / 8;
    offset     = offset % 8;

    // Start with last byte written. This code will only run when a write spans multiple bytes
    if (length >= (8 - offset))
    {
        // Calculate the number of bits the final byte read will use
        uint8_t const endLength = static_cast<uint8_t>((length + offset - 8) % 8);
        if (endLength != 0)
        {
            // set lowest bits to 1
            uint8_t const srcEndMask = ::std::numeric_limits<uint8_t>::max() >> (8 - endLength);
            // set highest bits to 1
            uint8_t const dstEndMask = ::std::numeric_limits<uint8_t>::max() << (8 - endLength);

            // Clear the bits we'll be setting, in case there's anything non-zero there
            dst[idx] &= ~dstEndMask;
            // Mask src, then shift over to correct position and bitwise or into dest
            dst[idx] |= ((static_cast<uint8_t>(src) & srcEndMask) << (8 - endLength));
            src >>= endLength;
            length -= endLength;
        }
        --idx;
    }

    while (length != 0)
    {
        if (length >= 8)
        {
            // Write a full byte at a time
            dst[idx] = static_cast<uint8_t>(src) & 0xFFU;
            if (sizeof(T) > 1)
            {
                src >>= 8;
            }
            --idx;
            length -= 8;
        }
        else
        {
            // Write the first byte
            // length < 8 guaranteed (by above conditional)
            uint8_t dstStartMask        = ::std::numeric_limits<uint8_t>::max() >> length;
            // Flip dstStartMask to set highest bits, shift by offset to shift mask into correct
            // starting position
            dstStartMask                = static_cast<uint8_t>(~dstStartMask) >> offset;
            // srcStartMask is just a mask for the lowest "length" # of bits in src
            uint8_t const srcStartMask  = ::std::numeric_limits<uint8_t>::max() >> (8 - length);
            uint8_t const dstStartShift = 8U - static_cast<uint8_t>(offset + length);

            // Clear lowest starting bits in case they contain anything non-zero
            dst[idx] &= ~dstStartMask;
            // Mask, shift, and or the src into the dst
            dst[idx] |= (static_cast<uint8_t>(src) & srcStartMask) << dstStartShift;
            // Same as length -= length
            length = 0;
        }
    }
}

} // namespace estd

#endif // GUARD_F2437631_4008_4301_B9A3_62725EA4B459
