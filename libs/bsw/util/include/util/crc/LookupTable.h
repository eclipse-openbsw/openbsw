// Copyright 2024 Accenture.

#ifndef GUARD_C600C7C8_E466_4410_925A_88EEB0C58ABF
#define GUARD_C600C7C8_E466_4410_925A_88EEB0C58ABF

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

#endif /* GUARD_C600C7C8_E466_4410_925A_88EEB0C58ABF */
