// Copyright 2024 Accenture.

/**
 * Contains estd::string and estd::declare::string
 * \file
 * \ingroup estl_containers
 */

#ifndef GUARD_0552EBDC_85B0_4451_AF80_D2DA8FA08525
#define GUARD_0552EBDC_85B0_4451_AF80_D2DA8FA08525

#include "estd/limits.h"
#include "estd/math.h"
#include "estd/result.h"
#include "estd/slice.h"

#include <cstring>

namespace estd
{
template<typename T, T v = 0>
struct guaranteePrimitive
{};

enum class ConvertError : uint8_t
{
    InvalidToken = 0U,
    InvalidBase  = 1U,
    Overflow     = 2U
}; // namespace estd

/*
 * Convert a string as a slice to an integer of type T, where T must be a primitive number type.
 * Slices must not have a null-terminating character.
 */
template<typename T>
::estd::result<T, ConvertError> strtonum(::estd::slice<uint8_t const> const data, uint8_t base = 10)
{
    (void)guaranteePrimitive<T>();

    uint64_t value      = 0;
    size_t index        = 0;
    bool const negation = (std::is_signed<T>::value && (data[0] == static_cast<uint8_t>('-')));
    if (negation || (data[0] == static_cast<uint8_t>('+')))
    {
        index++;
    }

    // Alphabetic mask of ASCII is & 0xDF (11011111), even after negating '0'
    constexpr uint8_t alphaMask = static_cast<uint8_t>(223);
    if ((base == 16) || (base == 0))
    {
        if ((index + 2U < data.size())
            && ((data[index + 1U] & alphaMask) == static_cast<uint8_t>('X')))
        {
            index += 2;
            base = 16;
        }
        else if (base == 0)
        {
            base = 10;
        }
        else
        {
            // Base == 16 but not 0x/0X
        }
    }
    else if ((base < 2) || (base > 36))
    {
        return ConvertError::InvalidBase;
    }
    else
    {
        // Base is set.
    }

    if (((data.size()) == 0U) || (data.size() - index == 0U))
    {
        // Token does not include a number
        return ConvertError::InvalidToken;
    }
    // Clear leading zeros, if loops til data.size() == index, returns 0 (as it was all zeros)
    while ((index < data.size()) && (data[index] == static_cast<uint8_t>('0')))
    {
        index++;
    }
    uint64_t const maxValue    = negation ? ::estd::abs<uint64_t>(std::numeric_limits<T>::min())
                                          : static_cast<uint64_t>(std::numeric_limits<T>::max());
    // Multiplication limit for any base is the integer division of the max value and base
    uint64_t const mulLimit    = maxValue / static_cast<uint64_t>(base);
    // For base X, the limit for adding is in the range 0:X-1
    uint8_t addLimit           = base - 1;
    constexpr uint8_t zeroChar = static_cast<uint8_t>('0');
    constexpr uint8_t aChar    = static_cast<uint8_t>('A');
    for (; index < data.size(); index++)
    {
        if (value >= mulLimit)
        {
            if (value != mulLimit)
            {
                return ConvertError::Overflow;
            }
            // If Value == mulLimit, the last add limit is the remainder of the integer division
            // done while calculating the multiplication limit.
            addLimit = static_cast<uint8_t>(maxValue % base);
        }
        uint8_t character = data[index] - zeroChar;

        if (character > 9)
        {
            character = (character & alphaMask) - (aChar - zeroChar - 10);
            if (character < 10)
            {
                return ConvertError::InvalidToken;
            }
        }

        if (character > addLimit)
        {
            // If the addLimit has changed in the last mulLimit check, this could be an overflow, so
            // check if the token is within the allowed chars of the base. Otherwise an invalid
            // token.
            return (character >= (base)) ? ConvertError::InvalidToken : ConvertError::Overflow;
        }
        value = (value * base) + character;
    }

    T const returnVal = ((negation) ? static_cast<T>(-value) : static_cast<T>(value));
    // If signed type, but positive, the cast of abs(minVal) results in a negative number, and thus
    // overflow as abs(minVal) == maxValue + 1.
    if (negation == (returnVal >= 0))
    {
        return ConvertError::Overflow;
    }
    return returnVal;
}

/*
 * Convert a char string to an integer of type T, where T must be a primitive number type.
 * Char strings must have null-terminating characters.
 */
template<typename T>
::estd::result<T, ConvertError> strtonum(char const* const str, uint8_t const base = 10)
{
    return strtonum<T>(::estd::make_str(str), base);
}

inline size_t strnlen(char const* const str, size_t const maxLen)
{
    size_t i = 0;
    while ((i < maxLen) && (str[i] != '\0'))
    {
        ++i;
    }
    return i;
}

class string
{
public:
    static constexpr size_t npos = ::estd::numeric_limits<size_t>::max;

    string(string const&) = default;
    /**
     * Copies the data from other to this string, replacing any data already in
     * this string. This function requires that there is room to copy the contents
     * of other into this string.
     *
     * \param other The other string to copy into this string.
     * \return a reference to this string.
     *
     * \assert{max_size() > other.size()}
     */
    string& operator=(string const& other);

    /**
     * Copies the data from the null-terminated str value to this string, replacing any data already
     * in this string. This function requires that there is room to copy the contents of 'str' into
     * this string.
     *
     * \param str A null-terminated string
     * \return a reference to this string.
     *
     * \assert{max_size() > strlen(str)}
     */
    string& operator=(char const* str);

    /**
     * Assigns the single character 'c' to this string. This string will be null-terminated.
     *
     * \param c A single character
     * \return a reference to this string.
     *
     * \assert{max_size() > 1}
     */
    string& operator=(char c);

    string& operator+=(string const& other);
    string& operator+=(char const* str);
    string& operator+=(char c);

    char& operator[](size_t const pos) { return _data[pos]; }

    char const& operator[](size_t const pos) const { return _data[pos]; }

    char& at(size_t pos);
    char const& at(size_t pos) const;

    size_t size() const { return _size; }

    size_t length() const { return _size; }

    size_t max_size() const { return _data.size(); }

    bool empty() const { return _size == 0U; }

    bool full() const { return _size == (_data.size() - 1U); }

    void resize(size_t const n) { resize(n, '\0'); }

    void resize(size_t n, char c);

    void clear();

    string& append(string const& other);
    string& append(string const& other, size_t subpos, size_t sublen = npos);
    string& append(char const* str);
    string& append(char const* str, size_t n);
    string& append(size_t n, char c);

    string& assign(string const& other);
    string& assign(string const& other, size_t subpos, size_t sublen = npos);
    string& assign(char const* str);
    string& assign(char const* str, size_t n);
    string& assign(size_t n, char c);

    char const* c_str() const { return _data.data(); }

    char const* data() const { return _data.data(); }

protected:
    explicit string(slice<char> const buffer) : _data(buffer) {}

private:
    slice<char> _data;
    size_t _size = 0U;
};

bool operator==(string const& lhs, string const& rhs);
bool operator==(string const& lhs, char const* rhs);
bool operator==(char const* lhs, string const& rhs);

bool operator<(string const& lhs, string const& rhs);
bool operator<(string const& lhs, char const* rhs);
bool operator<(char const* lhs, string const& rhs);

bool operator!=(string const& lhs, string const& rhs);
bool operator!=(string const& lhs, char const* rhs);
bool operator!=(char const* lhs, string const& rhs);

bool operator>(string const& lhs, string const& rhs);
bool operator>(string const& lhs, char const* rhs);
bool operator>(char const* lhs, string const& rhs);

bool operator<=(string const& lhs, string const& rhs);
bool operator<=(string const& lhs, char const* rhs);
bool operator<=(char const* lhs, string const& rhs);

bool operator>=(string const& lhs, string const& rhs);
bool operator>=(string const& lhs, char const* rhs);
bool operator>=(char const* lhs, string const& rhs);

namespace declare
{
template<size_t N>
class string : public ::estd::string
{
public:
    using base_type = ::estd::string;
    using this_type = ::estd::declare::string<N>;

    string() : ::estd::string(slice<char, N>(_realData)), _realData() {}

    explicit string(base_type const& other) : ::estd::string(slice<char, N>(_realData))
    {
        base_type::operator=(other);
    }

    string(this_type const& other) : ::estd::string(slice<char, N>(_realData))
    {
        base_type::operator=(other);
    }

    string(char const* const data) : ::estd::string(slice<char, N>(_realData))
    {
        base_type::operator=(data);
    }

    string(size_t const n, char const c) : ::estd::string(slice<char, N>(_realData))
    {
        (void)assign(n, c);
    }

    ~string() = default;

    string& operator=(base_type const& other)
    {
        base_type::operator=(other);
        return *this;
    }

    string& operator=(this_type const& other)
    {
        base_type::operator=(other);
        return *this;
    }

    string& operator=(char const* const data)
    {
        base_type::operator=(data);
        return *this;
    }

    string& operator=(char const c)
    {
        base_type::operator=(c);
        return *this;
    }

private:
    char _realData[N];
};

} // namespace declare

inline bool operator==(char const* const lhs, string const& rhs) { return operator==(rhs, lhs); }

inline bool operator!=(string const& lhs, string const& rhs) { return !(lhs == rhs); }

inline bool operator!=(char const* const lhs, string const& rhs) { return !(lhs == rhs); }

inline bool operator!=(string const& lhs, char const* const rhs) { return !(lhs == rhs); }

inline bool operator>(string const& lhs, string const& rhs) { return (rhs < lhs); }

inline bool operator>(char const* const lhs, string const& rhs) { return (rhs < lhs); }

inline bool operator>(string const& lhs, char const* const rhs) { return (rhs < lhs); }

inline bool operator<=(string const& lhs, string const& rhs) { return !(lhs > rhs); }

inline bool operator<=(char const* const lhs, string const& rhs) { return !(lhs > rhs); }

inline bool operator<=(string const& lhs, char const* const rhs) { return !(lhs > rhs); }

inline bool operator>=(string const& lhs, string const& rhs) { return !(rhs > lhs); }

inline bool operator>=(char const* const lhs, string const& rhs) { return !(rhs > lhs); }

inline bool operator>=(string const& lhs, char const* const rhs) { return !(rhs > lhs); }

/*
 * Convert an ::estd::string to an integer of type T, where T must be a primitive number type.
 */
template<typename T>
::estd::result<T, ConvertError> strtonum(::estd::string const string, uint8_t const base = 10)
{
    ::estd::slice<char const> const string_slice(string);
    return strtonum<T>(string_slice.reinterpret_as<uint8_t const>(), base);
}

} // namespace estd

#endif /* GUARD_0552EBDC_85B0_4451_AF80_D2DA8FA08525 */
