// Copyright 2024 Accenture.

#pragma once

#include <estd/algorithm.h>
#include <estd/assert.h>
#include <platform/estdint.h>

#include <limits>

namespace estd
{
template<typename T>
T read_le(uint8_t const*);

template<typename T>
void write_le(uint8_t*, T);

template<>
inline uint8_t read_le<uint8_t>(uint8_t const* const s)
{
    return s[0];
}

template<>
inline uint16_t read_le<uint16_t>(uint8_t const* const s)
{
    return static_cast<uint16_t>(static_cast<uint16_t>(s[1]) << 8U) | static_cast<uint16_t>(s[0]);
}

template<>
inline uint32_t read_le<uint32_t>(uint8_t const* const s)
{
    return static_cast<uint32_t>(s[0]) | (static_cast<uint32_t>(s[1]) << 8U)
           | (static_cast<uint32_t>(s[2]) << 16U) | (static_cast<uint32_t>(s[3]) << 24U);
}

template<>
inline uint64_t read_le<uint64_t>(uint8_t const* const s)
{
    return static_cast<uint64_t>(s[0]) | (static_cast<uint64_t>(s[1]) << 8U)
           | (static_cast<uint64_t>(s[2]) << 16U) | (static_cast<uint64_t>(s[3]) << 24U)
           | (static_cast<uint64_t>(s[4]) << 32U) | (static_cast<uint64_t>(s[5]) << 40U)
           | (static_cast<uint64_t>(s[6]) << 48U) | (static_cast<uint64_t>(s[7]) << 56U);
}

template<>
inline int8_t read_le<int8_t>(uint8_t const* const s)
{
    return static_cast<int8_t>(read_le<uint8_t>(s));
}

template<>
inline int16_t read_le<int16_t>(uint8_t const* const s)
{
    return static_cast<int16_t>(read_le<uint16_t>(s));
}

template<>
inline int32_t read_le<int32_t>(uint8_t const* const s)
{
    return static_cast<int32_t>(read_le<uint32_t>(s));
}

template<>
inline int64_t read_le<int64_t>(uint8_t const* const s)
{
    return static_cast<int64_t>(read_le<uint64_t>(s));
}

template<>
inline void write_le<uint8_t>(uint8_t* const s, uint8_t const value)
{
    s[0] = value;
}

template<>
inline void write_le<uint16_t>(uint8_t* const s, uint16_t const value)
{
    s[0] = static_cast<uint8_t>(value);
    s[1] = static_cast<uint8_t>(value >> 8U);
}

template<>
inline void write_le<uint32_t>(uint8_t* const s, uint32_t const value)
{
    s[0] = static_cast<uint8_t>(value);
    s[1] = static_cast<uint8_t>(value >> 8U);
    s[2] = static_cast<uint8_t>(value >> 16U);
    s[3] = static_cast<uint8_t>(value >> 24U);
}

template<>
inline void write_le<uint64_t>(uint8_t* const s, uint64_t const value)
{
    s[0] = static_cast<uint8_t>(value);
    s[1] = static_cast<uint8_t>(value >> 8U);
    s[2] = static_cast<uint8_t>(value >> 16U);
    s[3] = static_cast<uint8_t>(value >> 24U);
    s[4] = static_cast<uint8_t>(value >> 32U);
    s[5] = static_cast<uint8_t>(value >> 40U);
    s[6] = static_cast<uint8_t>(value >> 48U);
    s[7] = static_cast<uint8_t>(value >> 56U);
}

template<>
inline void write_le<int8_t>(uint8_t* const s, int8_t const value)
{
    write_le<uint8_t>(s, static_cast<uint8_t>(value));
}

template<>
inline void write_le<int16_t>(uint8_t* const s, int16_t const value)
{
    write_le<uint16_t>(s, static_cast<uint16_t>(value));
}

template<>
inline void write_le<int32_t>(uint8_t* const s, int32_t const value)
{
    write_le<uint32_t>(s, static_cast<uint32_t>(value));
}

template<>
inline void write_le<int64_t>(uint8_t* const s, int64_t const value)
{
    write_le<uint64_t>(s, static_cast<uint64_t>(value));
}

template<typename T>
struct little_endian
{
    uint8_t bytes[sizeof(T)];

    little_endian& operator=(T const value)
    {
        write_le(&bytes[0], value);
        return *this;
    }

    operator T() const { return read_le<T>(&bytes[0]); }

    static little_endian make(T const value)
    {
        little_endian ret;
        ret = value;
        return ret;
    }
};

template<typename T>
inline little_endian<T> make_le(T const value)
{
    little_endian<T> ret;
    ret = value;
    return ret;
}

using le_uint16_t = little_endian<uint16_t>;
using le_uint32_t = little_endian<uint32_t>;
using le_uint64_t = little_endian<uint64_t>;

using le_int16_t = little_endian<int16_t>;
using le_int32_t = little_endian<int32_t>;
using le_int64_t = little_endian<int64_t>;

inline uint32_t read_le_24(uint8_t const* const s)
{
    return static_cast<uint32_t>(s[0]) | (static_cast<uint32_t>(s[1]) << 8U)
           | (static_cast<uint32_t>(s[2]) << 16U);
}

inline void write_le_24(uint8_t* const s, uint32_t const value)
{
    s[0] = static_cast<uint8_t>(value >> 0U);
    s[1] = static_cast<uint8_t>(value >> 8U);
    s[2] = static_cast<uint8_t>(value >> 16U);
}

inline uint64_t read_le_48(uint8_t const* const s)
{
    return static_cast<uint64_t>(s[0]) | (static_cast<uint64_t>(s[1]) << 8U)
           | (static_cast<uint64_t>(s[2]) << 16U) | (static_cast<uint64_t>(s[3]) << 24U)
           | (static_cast<uint64_t>(s[4]) << 32U) | (static_cast<uint64_t>(s[5]) << 40U);
}

inline void write_le_48(uint8_t* const s, uint64_t const value)
{
    s[0] = static_cast<uint8_t>(value >> 0U);
    s[1] = static_cast<uint8_t>(value >> 8U);
    s[2] = static_cast<uint8_t>(value >> 16U);
    s[3] = static_cast<uint8_t>(value >> 24U);
    s[4] = static_cast<uint8_t>(value >> 32U);
    s[5] = static_cast<uint8_t>(value >> 40U);
}

/**
 * @brief
 * Read LE bits from "right to left" (highest index read first), and shift read result right with
 * each further read
 * \tparam T Returning integral type
 */
template<typename T>
T read_le_bits(uint8_t const* const src, size_t offset, size_t length)
{
    estd_assert(length <= sizeof(T) * 8);
    T result   = 0;
    // Adjust offset to modulo of 8 for ease of calculation, and set index accordingly
    size_t idx = (offset + length) / 8;
    offset     = offset % 8;

    // Start with last byte written. This code will only run when a write spans multiple bytes
    if (length >= (8 - offset))
    {
        // Calculate the number of bits the final byte read will use
        uint8_t const endLength  = static_cast<uint8_t>(length + offset - 8) % 8U;
        uint8_t const endShift   = 8 - endLength;
        // set lowest bits to 1
        uint8_t const srcEndMask = ::std::numeric_limits<uint8_t>::max() >> endShift;

        // Mask src, then shift over to correct position and bitwise or into result
        result = static_cast<T>((src[idx] >> endShift) & srcEndMask);
        --idx;
        length -= endLength;
    }

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
            --idx;
            length -= 8;
        }
        else
        {
            // Final (first) partial byte read
            uint8_t const startShift
                = 8U - ::estd::min<uint8_t>(8U, static_cast<uint8_t>(offset + length));

            // srcStartMask is just a mask for the lowest "length" # of bits in src
            uint8_t const startMask = ::std::numeric_limits<uint8_t>::max() >> (8 - length);
            result <<= length;
            // Mask, shift, and or the src into the dst
            result |= static_cast<T>((src[idx] >> startShift) & startMask);
            // Same as length -= length
            length = 0;
        }
    }
    return result;
}

/**
 * @brief
 * Write LE bits from "right to left" (highest index written first), and shift input source right
 * with each further write
 * \tparam T Source integral type
 */
template<typename T>
void write_le_bits(uint8_t* const dst, T src, size_t offset, size_t length)
{
    estd_assert(length <= sizeof(T) * 8);
    // Adjust offset to modulo of 8 for ease of calculation, and set index accordingly
    size_t idx                = offset / 8;
    offset                    = offset % 8;
    // Shift value for mask: as it's potentially only part of the first byte, min between length
    // and Number of bytes in first byte to read
    uint8_t const firstLength = static_cast<uint8_t>(::estd::min<size_t>(length, 8U - offset));
    // Shift value for return value: 8 - (offset + length), or 0 if offset + length >= 8
    uint8_t const firstShift  = 8U - static_cast<uint8_t>(::estd::min<size_t>(8U, offset + length));
    // Take uint8_t max (all 1s), shift by number of bits to read, then invert (all 0s for
    // length 0)
    uint8_t const firstSrcMask = ~(::std::numeric_limits<uint8_t>::max() << firstLength);
    uint8_t firstDstMask       = ~(::std::numeric_limits<uint8_t>::max() >> firstLength);
    firstDstMask >>= offset;

    dst[idx] &= firstDstMask;
    dst[idx] |= (static_cast<uint8_t>(src) & firstSrcMask) << firstShift;
    length -= firstLength;
    src >>= firstLength;
    ++idx;

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
            ++idx;
            length -= 8;
        }
        else
        {
            // length < 8 guaranteed (by above conditional)
            // Flip dstStartMask to set highest bits
            uint8_t const lastDstMask  = ~(::std::numeric_limits<uint8_t>::max() >> length);
            // starting position
            // lastSrcMask is just a mask for the lowest "length" # of bits in src
            uint8_t const lastDstShift = static_cast<uint8_t>(8U - length);
            uint8_t const lastSrcMask  = ::std::numeric_limits<uint8_t>::max() >> lastDstShift;

            // Clear lowest starting bits in case they contain anything non-zero
            dst[idx] &= ~lastDstMask;
            // Mask, shift, and or the src into the dst
            dst[idx] |= (static_cast<uint8_t>(src) & lastSrcMask) << lastDstShift;
            // Same as length -= length
            length = 0;
        }
    }
}

} // namespace estd
