// Copyright 2024 Accenture.

#ifndef GUARD_A0A9C9D5_F6E7_4C50_AB8A_70AD7597BF68
#define GUARD_A0A9C9D5_F6E7_4C50_AB8A_70AD7597BF68

#include <iterator>

namespace estd
{
template<class ForwardIt>
ForwardIt next(ForwardIt it, typename std::iterator_traits<ForwardIt>::difference_type const n = 1)
{
    std::advance(it, n);
    return it;
}

} // namespace estd

#endif // GUARD_A0A9C9D5_F6E7_4C50_AB8A_70AD7597BF68
