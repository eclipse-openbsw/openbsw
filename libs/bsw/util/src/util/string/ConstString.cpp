/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/string/ConstString.h"

#include <etl/char_traits.h>
#include <etl/memory.h>
#include <etl/string_view.h>

namespace util
{
namespace string
{
int32_t ConstString::compare(ConstString const& other) const
{
    size_t const compareLength = (_length < other._length) ? _length : other._length;
    int32_t result
        = (compareLength > 0U) ? ::etl::mem_compare(other._data, compareLength, _data) : 0;
    if (result == 0)
    {
        result = static_cast<int32_t>(_length) - static_cast<int32_t>(other._length);
    }
    return result;
}

int32_t ConstString::compareIgnoreCase(ConstString const& other) const
{
    size_t const compareLength = (_length < other._length) ? _length : other._length;
    ::etl::string_view const self(_data, _length);
    ::etl::string_view const rhs(other._data, other._length);
    int32_t result = 0;
    size_t i       = 0U;

    while ((result == 0) && (i < compareLength))
    {
        result = toLower(static_cast<int32_t>(self[i])) - toLower(static_cast<int32_t>(rhs[i]));
        ++i;
    }
    if (result == 0)
    {
        result = static_cast<int32_t>(_length) - static_cast<int32_t>(other._length);
    }
    return result;
}

int32_t ConstString::find(ConstString const& str, uint32_t const offset) const
{
    int32_t const NOT_FOUND = -1;
    if (static_cast<size_t>(offset) > _length)
    {
        return NOT_FOUND;
    }
    ::etl::string_view const haystack(_data, _length);
    ::etl::string_view const needle(str._data, str._length);
    size_t const pos = haystack.find(needle, static_cast<size_t>(offset));
    return (pos == ::etl::string_view::npos) ? NOT_FOUND : static_cast<int32_t>(pos);
}

bool ConstString::contains(ConstString const& str) const { return find(str) >= 0; }

} /* namespace string */
} /* namespace util */
