/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/array.h>

#include <cmath>

namespace someip
{
namespace types
{
template<class CODING_, class T, std::size_t N>
bool validateSimpleCoding(::etl::array<T, N> const& data)
{
    using simple_array_iterator = typename ::etl::array<T, N>::const_iterator;

    simple_array_iterator const endIter = data.cend();

    for (simple_array_iterator iter = data.cbegin(); iter < endIter; ++iter)
    {
        if (CODING_::validate(static_cast<float_t>(*iter)) == false)
        {
            return false;
        }
    }

    return true;
}

template<class T, std::size_t N>
bool validateComplexCoding(::etl::array<T, N> const& data)
{
    using complex_array_iterator         = typename ::etl::array<T, N>::const_iterator;
    complex_array_iterator const endIter = data.cend();

    for (complex_array_iterator iter = data.cbegin(); iter < endIter; ++iter)
    {
        if (iter->isValid() == false)
        {
            return false;
        }
    }

    return true;
}

} // namespace types
} // namespace someip
