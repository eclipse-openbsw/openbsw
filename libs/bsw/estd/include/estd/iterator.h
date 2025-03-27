// Copyright 2024 Accenture.

#pragma once

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
