// Copyright 2024 Accenture.

#pragma once

namespace util
{
namespace crc
{
template<class T, T Mask = T()>
struct Xor
{
    static inline T apply(T const data) { return data ^ Mask; }
};
} // namespace crc
} // namespace util

