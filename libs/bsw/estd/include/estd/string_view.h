// Copyright 2024 Accenture.

#pragma once

#include "estd/limits.h"
#include "estd/slice.h"

#include <cstring>

namespace estd
{

struct string_view_tokens;

class string_view final
{
public:
    using value_type      = char;
    using pointer         = char*;
    using const_pointer   = char const*;
    using reference       = char&;
    using const_reference = char const&;

    using const_iterator         = typename slice<char const>::const_iterator;
    using iterator               = const_iterator;
    using const_reverse_iterator = typename slice<char const>::const_reverse_iterator;
    using reverse_iterator       = const_reverse_iterator;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    static constexpr size_t npos = ::estd::numeric_limits<size_type>::max;

public:
    string_view() = default;
    string_view(char const* const s, size_type const count);
    explicit string_view(char const* const s);
    explicit string_view(std::nullptr_t)                        = delete;
    explicit string_view(std::nullptr_t, size_type const count) = delete;

    const_iterator begin() const;
    const_iterator cbegin() const;
    const_iterator end() const;
    const_iterator cend() const;
    const_reverse_iterator rbegin() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator rend() const;
    const_reverse_iterator crend() const;

    const_reference operator[](size_type const pos) const;
    const_reference at(size_type const pos) const;
    const_reference front() const;
    const_reference back() const;

    const_pointer data() const;
    size_type size() const;
    size_type length() const;
    size_type max_size() const;

    bool empty() const;

    void remove_prefix(size_type const n);
    void remove_suffix(size_type const n);

    int32_t compare(string_view const v) const;
    int32_t compare(size_type const pos1, size_type const count1, string_view const v) const;
    int32_t compare(
        size_type const pos1,
        size_type const count1,
        string_view const v,
        size_type const pos2,
        size_type const count2) const;
    int32_t compare(char const* const s) const;
    int32_t compare(
        size_type const pos1,
        size_type const count1,
        char const* const s,
        size_type const count2) const;
    int32_t compare(size_type const pos1, size_type const count1, char const* const s) const;

    void swap(string_view& in);
    size_type copy(pointer const dest, size_type count, size_type const pos = 0) const;
    string_view substr(size_type const pos = 0, size_type count = npos) const;

    bool starts_with(string_view const sv) const;
    bool starts_with(char const ch) const;
    bool starts_with(char const* const s) const;

    bool ends_with(string_view const sv) const;
    bool ends_with(char const ch) const;
    bool ends_with(char const* const s) const;

    size_type find(string_view const v, size_type const pos = 0) const;
    size_type find(char const ch, size_type const pos = 0) const;
    size_type find(char const* const s, size_type const pos = 0) const;
    size_type find(char const* const s, size_type const pos, size_type const count) const;

    size_type find_first_of(string_view const v, size_type const pos = 0) const;
    size_type find_first_of(char const* const s, size_type const pos = 0) const;

    size_type find_first_not_of(string_view const v, size_type const pos = 0) const;
    size_type find_first_not_of(char const* const s, size_type const pos = 0) const;

    string_view& skip_while_matches(string_view const v, size_type const pos = 0);
    string_view& skip_while_matches(char const* const s, size_type const pos = 0);

    string_view& skip_until_matches(string_view const v, size_type const pos = 0);
    string_view& skip_until_matches(char const* const s, size_type const pos = 0);

    template<typename T>
    string_view_tokens tokenize(T const& delims) const;

private:
    slice<char const> _data;
};

bool operator==(string_view const& lhs, string_view const& rhs);
bool operator!=(string_view const& lhs, string_view const& rhs);
bool operator<(string_view const& lhs, string_view const& rhs);
bool operator<=(string_view const& lhs, string_view const& rhs);
bool operator>(string_view const& lhs, string_view const& rhs);
bool operator>=(string_view const& lhs, string_view const& rhs);

/**
 * @brief Iterator over tokens of a string_view given a set of delimiters.
 * Example:
 * \code{.cpp}
 *     string_view view{"Hello World"};
 *     auto tokens view.tokenize(" ");
 *     for (auto token : tokens)
 *     {
 *         cout << token << endl;
 *     }
 *     // stdout:
 *     // Hello
 *     // World
 * \endcode
 */
struct string_view_tokens
{
    string_view_tokens() = default;

    string_view_tokens(string_view const& source, string_view const& delims) : _delims(delims)
    {
        auto first_token = source;
        first_token.skip_while_matches(_delims);

        _current_token = first_token;
        _next_token    = first_token;
        // advance into first token
        this->operator++();
    }

    string_view_tokens& operator++()
    {
        // Advance  current token to next token.
        _current_token = _next_token;
        // Find next delimiter
        _next_token.skip_until_matches(_delims);
        // trim current token up to next delimiter
        _current_token.remove_suffix(_next_token.length());
        // find start of next token
        _next_token.skip_while_matches(_delims);

        return *this;
    }

    bool operator==(string_view_tokens const& other) const
    {
        return (_current_token == other._current_token) && (_next_token == other._next_token);
    }

    bool operator!=(string_view_tokens const& other) const { return !(*this == other); }

    string_view const operator*() const { return _current_token; }

    string_view_tokens& begin() { return *this; }

    static string_view_tokens end() { return string_view_tokens(); }

private:
    string_view _current_token;
    string_view _next_token;
    // string_view has a slice which uses NSDMI; therefore we shouldn't need it here, but clang
    // deletes default constructor (-Wdefaulted-function-deleted) when it shouldn't
    string_view const _delims{};
};

template<typename T>
string_view_tokens string_view::tokenize(T const& delims) const
{
    return string_view_tokens(*this, string_view(delims));
}

} // namespace estd
