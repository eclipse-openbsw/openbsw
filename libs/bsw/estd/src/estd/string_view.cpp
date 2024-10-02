// Copyright 2024 Accenture.

#include "estd/string_view.h"

#include "estd/algorithm.h"
#include "estd/assert.h"

namespace estd
{

string_view::string_view(char const* const s, size_type const count)
: _data(::estd::slice<char const>::from_pointer(s, count))
{}

string_view::string_view(char const* const s)
: _data(::estd::slice<char const>::from_pointer(s, strlen(s)))
{}

string_view::const_iterator string_view::begin() const { return _data.begin(); }

string_view::const_iterator string_view::cbegin() const { return _data.begin(); }

string_view::const_iterator string_view::end() const { return _data.end(); }

string_view::const_iterator string_view::cend() const { return _data.end(); }

string_view::const_reverse_iterator string_view::rbegin() const { return _data.rbegin(); }

string_view::const_reverse_iterator string_view::crbegin() const { return _data.crbegin(); }

string_view::const_reverse_iterator string_view::rend() const { return _data.rend(); }

string_view::const_reverse_iterator string_view::crend() const { return _data.crend(); }

string_view::const_reference string_view::operator[](size_type const pos) const
{
    return _data[pos];
}

string_view::const_reference string_view::at(size_type const pos) const
{
    estd_assert(pos < _data.size());
    return _data.at(pos);
}

string_view::const_reference string_view::front() const { return *_data.begin(); }

string_view::const_reference string_view::back() const { return *_data.rbegin(); }

string_view::const_pointer string_view::data() const { return _data.data(); }

string_view::size_type string_view::size() const { return _data.size(); }

string_view::size_type string_view::length() const { return _data.size(); }

string_view::size_type string_view::max_size() const { return _data.size(); }

bool string_view::empty() const { return size() == 0U; }

void string_view::remove_prefix(size_type const n)
{
    estd_assert(n <= _data.size());
    _data.advance(n);
}

void string_view::remove_suffix(size_type const n)
{
    estd_assert(n <= _data.size());
    _data.trim(size() - n);
}

int32_t string_view::compare(string_view const v) const
{
    auto first  = begin();
    auto second = v.begin();
    while ((first != end()) && (second != v.end()))
    {
        if (*first != *second)
        {
            return static_cast<int32_t>(static_cast<uint8_t>(*first))
                   - static_cast<int32_t>(static_cast<uint8_t>(*second));
        }
        ++first;
        ++second;
    }
    return static_cast<int32_t>(_data.size()) - static_cast<int32_t>(v.size());
}

int32_t
string_view::compare(size_type const pos1, size_type const count1, string_view const v) const
{
    estd_assert(pos1 < _data.size());
    return substr(pos1, count1).compare(v);
}

int32_t string_view::compare(
    size_type const pos1,
    size_type const count1,
    string_view const v,
    size_type const pos2,
    size_type const count2) const
{
    estd_assert(pos1 < _data.size());
    estd_assert(pos2 < v.size());

    auto const sstr(substr(pos1, count1));
    auto const sstr2(v.substr(pos2, count2));

    return sstr.compare(sstr2);
}

int32_t string_view::compare(char const* const s) const { return compare(string_view(s)); }

int32_t string_view::compare(
    size_type const pos1, size_type const count1, char const* const s, size_type const count2) const
{
    estd_assert(pos1 < _data.size());

    return compare(pos1, count1, string_view(s), 0, count2);
}

int32_t
string_view::compare(size_type const pos1, size_type const count1, char const* const s) const
{
    estd_assert(pos1 < _data.size());

    return compare(pos1, count1, string_view(s));
}

void string_view::swap(string_view& in) { ::estd::swap(*this, in); }

string_view::size_type
string_view::copy(pointer const dest, size_type count, size_type const pos) const
{
    estd_assert(pos < _data.size());
    if (dest == nullptr)
    {
        return 0;
    }
    count = min(size() - pos, count);
    (void)memcpy(dest, &_data.at(pos), count);

    return count;
}

string_view string_view::substr(size_type const pos, size_type count) const
{
    estd_assert(pos < _data.size());

    count = min(size() - pos, count);
    return string_view(&_data.at(pos), count);
}

bool string_view::starts_with(string_view const sv) const
{
    if (_data.size() < sv.size())
    {
        return false;
    }

    auto const sstr = substr(0, sv.size());

    return sstr.compare(sv) == 0;
}

bool string_view::starts_with(char const ch) const { return (!empty()) && (front() == ch); }

bool string_view::starts_with(char const* const s) const { return starts_with(string_view(s)); }

bool string_view::ends_with(string_view const sv) const
{
    if (size() < sv.size())
    {
        return false;
    }

    return compare(size() - sv.size(), npos, sv) == 0;
}

bool string_view::ends_with(char const ch) const { return (!empty()) && (back() == ch); }

bool string_view::ends_with(char const* const s) const { return ends_with(string_view(s)); }

string_view::size_type string_view::find(string_view const v, size_type pos) const
{
    if (v.empty())
    {
        return 0;
    }

    if (empty())
    {
        return npos;
    }

    estd_assert(pos < size());
    estd_assert(v.size() <= size());

    // substr(pos, N) goes up to pos + (N-1), so range below can be inclusive
    for (; pos <= size() - v.size(); ++pos)
    {
        if (substr(pos, v.size()) == v)
        {
            return pos;
        }
    }
    return npos;
}

string_view::size_type string_view::find(char const ch, size_type const pos) const
{
    return find(string_view(&ch, 1), pos);
}

string_view::size_type string_view::find(char const* const s, size_type const pos) const
{
    return find(string_view(s), pos);
}

string_view::size_type
string_view::find(char const* const s, size_type const pos, size_type const count) const
{
    return find(string_view(s, count), pos);
}

string_view::size_type string_view::find_first_of(string_view const v, size_type pos) const
{
    if (v.empty() || empty())
    {
        return npos;
    }

    estd_assert(pos < _data.size());
    estd_assert(v.size() <= _data.size());

    for (; pos < _data.size(); ++pos)
    {
        if (v.find(_data[pos]) != string_view::npos)
        {
            return pos;
        }
    }
    return npos;
}

string_view::size_type string_view::find_first_of(char const* const s, size_type const pos) const
{
    return find_first_of(string_view(s), pos);
}

string_view::size_type string_view::find_first_not_of(string_view const v, size_type pos) const
{
    if (empty())
    {
        return npos;
    }

    if (v.empty())
    {
        return 0;
    }

    estd_assert(pos < _data.size());

    for (; pos < _data.size(); ++pos)
    {
        if (v.find(_data[pos]) == string_view::npos)
        {
            return pos;
        }
    }
    return npos;
}

string_view::size_type
string_view::find_first_not_of(char const* const s, size_type const pos) const
{
    return find_first_not_of(string_view(s), pos);
}

string_view& string_view::skip_while_matches(string_view const v, size_type const pos)
{
    remove_prefix(estd::min(find_first_not_of(v, pos), size()));
    return *this;
}

string_view& string_view::skip_while_matches(char const* const s, size_type const pos)
{
    return skip_while_matches(string_view(s), pos);
}

string_view& string_view::skip_until_matches(string_view const v, size_type const pos)
{
    remove_prefix(estd::min(find_first_of(v, pos), size()));
    return *this;
}

string_view& string_view::skip_until_matches(char const* const s, size_type const pos)
{
    return skip_until_matches(string_view(s), pos);
}

bool operator==(string_view const& lhs, string_view const& rhs) { return (lhs.compare(rhs) == 0); }

bool operator!=(string_view const& lhs, string_view const& rhs) { return !(operator==(lhs, rhs)); }

bool operator<(string_view const& lhs, string_view const& rhs) { return lhs.compare(rhs) < 0; }

bool operator<=(string_view const& lhs, string_view const& rhs)
{
    return (lhs < rhs || lhs == rhs);
}

bool operator>(string_view const& lhs, string_view const& rhs) { return !(lhs <= rhs); }

bool operator>=(string_view const& lhs, string_view const& rhs) { return !(lhs < rhs); }

} // namespace estd
