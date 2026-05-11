/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/limits.h>
#include <etl/type_traits.h>

#include <platform/estdint.h>

namespace routing
{
template<typename T>
struct ClampedCounter
{
    static_assert(
        ::etl::is_unsigned<T>::value && ::etl::is_integral<T>::value,
        "Value is not unsigned and not an integral type");

    using Type = T;

    ClampedCounter() = default;

    operator T() const { return count; }

    T& operator++() { return count < ::etl::numeric_limits<T>::max() ? ++count : count; }

    T operator++(int)
    {
        T const tmp = count;
        (void)this->operator++();
        return tmp;
    }

    T& operator+=(T const& n)
    {
        T const res = count + n;
        if (count > res)
        {
            count = ::etl::numeric_limits<T>::max();
        }
        else
        {
            count = res;
        }
        return count;
    }

    T count = 0;
};

} // namespace routing
