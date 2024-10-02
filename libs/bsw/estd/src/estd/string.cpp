// Copyright 2024 Accenture.

#include "estd/string.h"

#include "estd/assert.h"

#include <algorithm>
#include <cstring>

namespace estd
{
string& string::operator=(string const& other)
{
    // identical string
    if (_data.data() == other._data.data())
    {
        return *this;
    }

    estd_assert(_data.size() > other._size);

    (void)memcpy(_data.data(), other._data.data(), other._size);

    _size        = other._size;
    _data[_size] = '\0';

    return *this;
}

string& string::operator=(char const* const str)
{
    size_t const len = strlen(str);

    estd_assert(_data.size() > len);

    (void)memcpy(_data.data(), str, len);

    _size        = len;
    _data[_size] = '\0';

    return *this;
}

string& string::operator=(char const c)
{
    estd_assert(_data.size() > 1U);
    _data[0U] = c;
    _size++;
    _data[_size] = '\0';

    return *this;
}

string& string::operator+=(string const& other) { return append(other); }

string& string::operator+=(char const* const str) { return append(str); }

string& string::operator+=(char const c)
{
    estd_assert(_data.size() > (_size + 1U));
    _data[_size] = c;
    _size++;
    _data[_size] = '\0';
    return *this;
}

char& string::at(size_t const pos)
{
    estd_assert(pos <= _size);
    return _data[pos];
}

char const& string::at(size_t const pos) const
{
    estd_assert(pos <= _size);
    return static_cast<char const&>(_data[pos]);
}

void string::resize(size_t const n, char const c)
{
    estd_assert(n < max_size());

    if (n == _size)
    {
        return;
    }
    // resizing smaller, need to remove elements
    if (n < _size)
    {
        _size        = n;
        _data[_size] = '\0';
        return;
    }

    // adding elements
    for (size_t i = _size; i < n; ++i)
    {
        _data[i] = c;
    }

    _size        = n;
    _data[_size] = '\0';
}

void string::clear()
{
    _size        = 0U;
    _data[_size] = '\0';
}

string& string::append(string const& other)
{
    estd_assert(_data.size() > (_size + other._size));

    (void)memcpy(&_data[_size], other._data.data(), other._size);

    _size += other._size;
    _data[_size] = '\0';

    return *this;
}

string& string::append(string const& other, size_t const subpos, size_t const sublen)
{
    if (sublen == npos)
    {
        // copy from subpos to end of string
        auto const total = static_cast<size_t>(other._size - subpos);
        return append(&other._data[subpos], total);
    }

    if ((subpos + sublen) > other._size)
    {
        // copy from subpos to end of string
        auto const total = static_cast<size_t>(other._size - subpos);
        return append(&other._data[subpos], total);
    }

    estd_assert(_data.size() > (_size + sublen));

    (void)memcpy(&_data[_size], &(other._data[subpos]), sublen);

    _size += sublen;
    _data[_size] = '\0';

    return *this;
}

string& string::append(char const* const str)
{
    size_t const len = strlen(str);

    estd_assert(_data.size() > (_size + len));

    (void)memcpy(&_data[_size], str, len);

    _size += len;
    _data[_size] = '\0';

    return *this;
}

string& string::append(char const* const str, size_t const n)
{
    estd_assert(_data.size() > (_size + n));

    (void)memcpy(&_data[_size], str, n);

    _size += n;
    _data[_size] = '\0';

    return *this;
}

string& string::append(size_t const n, char const c)
{
    estd_assert(_data.size() > (_size + n));

    (void)memset(&_data[_size], static_cast<int>(c), n);

    _size += n;
    _data[_size] = '\0';

    return *this;
}

string& string::assign(string const& other)
{
    if (_data.data() == other._data.data())
    {
        return *this;
    }

    _size = 0U;

    return append(other);
}

string& string::assign(string const& other, size_t const subpos, size_t const sublen)
{
    estd_assert(_data.data() != other._data.data());

    _size = 0U;

    return append(other, subpos, sublen);
}

string& string::assign(char const* const str)
{
    _size = 0U;

    return append(str);
}

string& string::assign(char const* const str, size_t const n)
{
    _size = 0U;

    return append(str, n);
}

string& string::assign(size_t const n, char const c)
{
    _size = 0U;

    return append(n, c);
}

bool operator==(string const& lhs, string const& rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    return memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
}

bool operator==(string const& lhs, char const* const rhs)
{
    size_t const len = strlen(rhs);

    if (lhs.size() != len)
    {
        return false;
    }

    return memcmp(lhs.data(), rhs, lhs.size()) == 0;
}

bool operator<(string const& lhs, string const& rhs)
{
    return std::lexicographical_compare(lhs.data(), &lhs[lhs.size()], rhs.data(), &rhs[rhs.size()]);
}

bool operator<(char const* const lhs, string const& rhs)
{
    size_t const len = strlen(lhs);
    return std::lexicographical_compare(lhs, &lhs[len], rhs.data(), &rhs[rhs.size()]);
}

bool operator<(string const& lhs, char const* const rhs)
{
    size_t const len = strlen(rhs);
    return std::lexicographical_compare(lhs.data(), &lhs[lhs.size()], rhs, &rhs[len]);
}

} // namespace estd
