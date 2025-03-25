// Copyright 2024 Accenture.

#pragma once

#include <cstdint>

namespace util
{
namespace memory
{
namespace internal
{
// This causes certain types to be handled exacly the same
// clang-format off
    template<typename T> struct TypeMap {};
    template<> struct TypeMap<int8_t>             { using type = uint8_t; };
    template<> struct TypeMap<int16_t>            { using type = uint16_t; };
    template<> struct TypeMap<int32_t>            { using type = uint32_t; };
    template<> struct TypeMap<int64_t>            { using type = uint64_t; };

// clang-format on
} // namespace internal

template<typename T>
T reverseBits(T const value)
{
    typename internal::TypeMap<T>::type const ret
        = reverseBits<typename internal::TypeMap<T>::type>(
            static_cast<typename internal::TypeMap<T>::type>(value));
    return static_cast<T>(ret);
}

template<>
inline uint8_t reverseBits(uint8_t value)
{
    value = ((value >> 1U) & 0x55U) | (static_cast<uint8_t>(value << 1U) & 0xAAU);
    value = ((value >> 2U) & 0x33U) | (static_cast<uint8_t>(value << 2U) & 0xCCU);
    return (value >> 4U) | static_cast<uint8_t>(value << 4U);
}

template<>
inline uint16_t reverseBits(uint16_t value)
{
    value = ((value >> 1U) & 0x5555U) | (static_cast<uint16_t>(value << 1U) & 0xAAAAU);
    value = ((value >> 2U) & 0x3333U) | (static_cast<uint16_t>(value << 2U) & 0xCCCCU);
    value = ((value >> 4U) & 0x0F0FU) | (static_cast<uint16_t>(value << 4U) & 0xF0F0U);
    return (value >> 8U) | static_cast<uint16_t>(value << 8U);
}

template<>
inline uint32_t reverseBits(uint32_t value)
{
    value = ((value >> 1U) & 0x55555555U) | ((value << 1U) & 0xAAAAAAAAU);
    value = ((value >> 2U) & 0x33333333U) | ((value << 2U) & 0xCCCCCCCCU);
    value = ((value >> 4U) & 0x0F0F0F0FU) | ((value << 4U) & 0xF0F0F0F0U);
    value = ((value >> 8U) & 0x00FF00FFU) | ((value << 8U) & 0xFF00FF00U);
    return (value >> 16U) | (value << 16U);
}

template<>
inline uint64_t reverseBits(uint64_t value)
{
    value = ((value >> 1U) & 0x5555555555555555U) | ((value << 1U) & 0xAAAAAAAAAAAAAAAAU);
    value = ((value >> 2U) & 0x3333333333333333U) | ((value << 2U) & 0xCCCCCCCCCCCCCCCCU);
    value = ((value >> 4U) & 0x0F0F0F0F0F0F0F0FU) | ((value << 4U) & 0xF0F0F0F0F0F0F0F0U);
    value = ((value >> 8U) & 0x00FF00FF00FF00FFU) | ((value << 8U) & 0xFF00FF00FF00FF00U);
    value = ((value >> 16U) & 0x0000FFFF0000FFFFU) | ((value << 16U) & 0xFFFF0000FFFF0000U);
    return (value >> 32U) | (value << 32U);
}

} // namespace memory
} // namespace util
