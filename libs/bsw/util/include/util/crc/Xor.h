// Copyright 2024 Accenture.

#ifndef GUARD_99E0D26B_C149_42A1_8DCF_ED5B85771501
#define GUARD_99E0D26B_C149_42A1_8DCF_ED5B85771501

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

#endif /* GUARD_99E0D26B_C149_42A1_8DCF_ED5B85771501 */
