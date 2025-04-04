// Copyright 2024 Accenture.

#pragma once

#include "util/memory/Bit.h"

namespace util
{
namespace crc
{
template<bool Reverse = false>
struct Reflect
{
    template<class T>
    static inline T apply(T const data)
    {
        return data;
    }
};

template<>
struct Reflect<true>
{
    template<class T>
    static inline T apply(T const data)
    {
        return ::util::memory::reverseBits(data);
    }
};

} // namespace crc
} // namespace util
