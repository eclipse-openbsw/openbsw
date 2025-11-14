// Copyright 2024 Accenture.

#include "util/string/ConstString.h"

#include <etl/char_traits.h>
#include <etl/memory.h>

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
    int32_t result             = 0;
    size_t i                   = 0U;

    while ((result == 0) && (i < compareLength))
    {
        result = toLower(static_cast<int32_t>(_data[i]))
                 - toLower(static_cast<int32_t>(other._data[i]));
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
    ConstString const substring(_data + offset);
    if (substring._length < str._length)
    {
        return NOT_FOUND;
    }
    size_t idx = 0U;
    while ((idx + str._length) <= substring._length)
    {
        if (::etl::strncmp(substring._data + idx, str._data, str._length) == ConstString::IS_EQUAL)
        {
            return static_cast<int32_t>(idx) + static_cast<int32_t>(offset);
        }
        ++idx;
    }
    return NOT_FOUND;
}

bool ConstString::contains(ConstString const& str) const { return find(str) >= 0; }

} /* namespace string */
} /* namespace util */
