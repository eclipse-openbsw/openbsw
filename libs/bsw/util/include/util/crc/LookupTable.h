// Copyright 2024 Accenture.

#pragma once

namespace util
{
namespace crc
{
// FIXME: add a static assert for the non specialized variant
//        so the error message shows that the crc table is missing!

template<class T, T Polynom>
struct LookupTable
{
    using TableRef = T const (&)[256];
    static TableRef getTable();
};

} // namespace crc
} // namespace util
