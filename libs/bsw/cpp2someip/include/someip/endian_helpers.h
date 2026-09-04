/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/algorithm.h>
#include <etl/binary.h>
#include <etl/endianness.h>
#include <cstdint>

namespace someip
{
namespace endian
{

// Read big-endian value from byte buffer
template<typename T>
inline T read_be(uint8_t const* ptr)
{
    T value;
    ::etl::copy(ptr, ptr + sizeof(T), reinterpret_cast<uint8_t*>(&value));
    if (::etl::endianness::value() == ::etl::endian::little)
    {
        value = ::etl::reverse_bytes(value);
    }
    return value;
}

// Write big-endian value to byte buffer
template<typename T>
inline void write_be(uint8_t* ptr, T value)
{
    if (::etl::endianness::value() == ::etl::endian::little)
    {
        value = ::etl::reverse_bytes(value);
    }
    ::etl::copy(
        reinterpret_cast<uint8_t const*>(&value),
        reinterpret_cast<uint8_t const*>(&value) + sizeof(T),
        ptr);
}

// Read 24-bit big-endian value
inline uint32_t read_be_24(uint8_t const* ptr)
{
    return (static_cast<uint32_t>(ptr[0]) << 16U) | (static_cast<uint32_t>(ptr[1]) << 8U)
           | static_cast<uint32_t>(ptr[2]);
}

// Write 24-bit big-endian value
inline void write_be_24(uint8_t* ptr, uint32_t value)
{
    ptr[0] = static_cast<uint8_t>(value >> 16U);
    ptr[1] = static_cast<uint8_t>(value >> 8U);
    ptr[2] = static_cast<uint8_t>(value);
}

// Read little-endian value from byte buffer
template<typename T>
inline T read_le(uint8_t const* ptr)
{
    T value;
    ::etl::copy(ptr, ptr + sizeof(T), reinterpret_cast<uint8_t*>(&value));
    if (::etl::endianness::value() == ::etl::endian::big)
    {
        value = ::etl::reverse_bytes(value);
    }
    return value;
}

// Write little-endian value to byte buffer
template<typename T>
inline void write_le(uint8_t* ptr, T value)
{
    if (::etl::endianness::value() == ::etl::endian::big)
    {
        value = ::etl::reverse_bytes(value);
    }
    ::etl::copy(
        reinterpret_cast<uint8_t const*>(&value),
        reinterpret_cast<uint8_t const*>(&value) + sizeof(T),
        ptr);
}

} // namespace endian
} // namespace someip
