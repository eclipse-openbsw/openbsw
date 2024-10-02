// Copyright 2024 Accenture.

#ifndef GUARD_2972EA04_C9D3_4B2B_8C2C_48A471E5E7A9
#define GUARD_2972EA04_C9D3_4B2B_8C2C_48A471E5E7A9

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

#endif /* GUARD_2972EA04_C9D3_4B2B_8C2C_48A471E5E7A9 */
