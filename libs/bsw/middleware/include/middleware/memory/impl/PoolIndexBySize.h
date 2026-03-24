// Copyright 2025 BMW AG

#pragma once

#include "middleware/memory/Pool.h"

#include <etl/tuple.h>

#include <cstdint>

namespace middleware
{
namespace memory
{
namespace impl
{

template<uint32_t N, int32_t I, typename Tuple>
struct PoolIndexBySizeImpl
{
    static int32_t const value
        = (etl::tuple_element<etl::tuple_size<Tuple>::value - I, Tuple>::type::valueSize() < N)
              ? PoolIndexBySizeImpl<N, I - 1, Tuple>::value
              : static_cast<int32_t>(etl::tuple_size<Tuple>::value - I);
};

template<uint32_t N, typename Tuple>
struct PoolIndexBySizeImpl<N, 0, Tuple>
{
    static int32_t const value = -1;
};

template<uint32_t N, typename Tuple>
struct PoolIndexBySize
{
    static int32_t const value
        = (etl::tuple_element<0, Tuple>::type::valueSize() < N)
              ? PoolIndexBySizeImpl<N, etl::tuple_size<Tuple>::value - 1, Tuple>::value
              : 0;
};

} // namespace impl
} // namespace memory
} // namespace middleware
