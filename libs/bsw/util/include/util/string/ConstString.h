// Copyright 2024 Accenture.

#pragma once

#ifndef ESR_NO_BUFFER_ASSERT
#include <util/estd/assert.h>
#endif

#include <cstdint>
#include <cstring>

namespace util
{
namespace string
{
/**
 * Structure used to represent a sized string.
 */
struct PlainSizedString
{
    /**
     * Pointer to the start of the data block wrapped by this ConstString object.
     */
    char const* _data;
    /**
     * Number of valid characters.
     */
    size_t _length;
};

/**
 * A ConstString is a lightweight wrapper around a const character buffer, which provides length
 * information and comparison operations.
 */
class ConstString : private PlainSizedString
{
public:
    /**
     * Creates an invalid buffer with the size 0.
     */
    ConstString();

    /**
     * Creates a new ConstByteBuffer for the provided data and the length.
     *
     * \param data pointer to the data block which shall be wrapped by this Buffer object.
     * \param length number of characters.
     */
    ConstString(char const* str, size_t len);

    /**
     * Creates a new ConstByteBuffer for the provided C string (terminating with zero).
     * The length will be calculated calling strlen on this string.
     *
     * \param string pointer to the zero terminated C string.
     */
    explicit ConstString(char const* str);

    /**
     * Creates a shallow copy of the provided character buffer.
     *
     * \param other buffer which shall be copied.
     *
     * \attention creates a shallow copy, the created object also will be a wrapper around
     *            the same data block.
     */
    ConstString(ConstString const& other);

    /**
     * Assignment operator.
     *
     * \param other whose data block and size will be assigned to this buffer.
     */
    ConstString& operator=(ConstString const& other);

    /**
     * Check for emptiness.
     *
     * \return
     * - true if empty
     * - false otherwise
     */
    bool isEmpty() const;

    /**
     * Returns the length of the string.
     *
     * \return number of valid characters
     */
    size_t length() const;

    /**
     * Index operator to emulate raw read access to the wrapped string.
     *
     * \param n Index of the data block which shall be accessed.
     *
     * \return a reference to the data element at the specified index.
     */
    char operator[](size_t n) const;

    /**
     * Compares with another string.
     *
     * \param other string to compare with
     * \param ignoreCase flag indicating whether case should be ignored or not.
     *
     * \return
     * - < 0 if the first character that does not match has a lower value in this than in other
     * - 0 the contents of both strings are equal
     * - > 0 if the first character that does not match has as higher value in this than in other
     */
    int32_t compare(ConstString const& other, bool ignoreCase) const;

    /**
     * Compares with another string.
     *
     * \param other string to compare with
     *
     * \return
     * - < 0 if the first character that does not match has a lower value in this than in other
     * - 0 the contents of both strings are equal
     * - > 0 if the first character that does not match has as higher value in this than in other
     */
    int32_t compare(ConstString const& other) const;

    /**
     * Compares with another string ignoring cases.
     *
     * \param other string to compare with
     *
     * \return
     * - < 0 if the first character that does not match has a lower value in this than in other
     * - 0 the contents of both strings are equal
     * - > 0 if the first character that does not match has as higher value in this than in other
     */
    int32_t compareIgnoreCase(ConstString const& other) const;

    /**
     * Return the contained character buffer.
     * \note Handle with care! The contained character buffer doesn't necessarily contain a
     * zero terminated C String!
     *
     * \return pointer to the character buffer
     */
    char const* data() const;

    /**
     * Return as pointer to a plain sized string.
     * \note This is intended mainly for usage when formatting output with
     * ::util::format::PrintfFormatter %S format
     *
     * \return pointer to the plain const string structure
     */
    PlainSizedString const* plain_str() const;

    /**
     * Return as pointer to a sized string. This can be used for
     * \note
     */

    /**
     * Finds the first occurrence of the provided string within this string.
     *
     * \param string for which will be searched.
     * \param offset where the search shall be started within this string (default: 0U).
     *
     * \return the offset to the from the start of this string to the first occurrence of the
     *         provided sting, if the string wasn't found at all a negative vale will be returned.
     */
    int32_t find(ConstString const& str, uint32_t offset = 0U) const;

    /**
     * Checks whether or not the specified string is contained in this string.
     *
     * \param string which will be searched for.
     *
     * \return <code>true</code> if the string is contained in this string,
     *         otherwise <code>false</code>.
     */
    bool contains(ConstString const& str) const;

private:
    static int32_t const IS_EQUAL = 0;

    /**
     * Convert ASCII uppercase letter to lower case.
     *
     * \param c character to convert
     *
     * \return lower case ASCII letter if c is a letter otherwise c
     */
    static int32_t toLower(int32_t c);
};

/**
 * Compares two constant string objects.
 * \param lhs ConstString object on the left-hand side of the operator
 * \param rhs ConstString object the right-hand side of the operator
 *  - true if both strings are equal.
 *  - false otherwise
 */
bool operator==(ConstString const& lhs, ConstString const& rhs);
/**
 * Compares two constant string objects.
 * \param lhs ConstString object on the left-hand side of the operator
 * \param rhs ConstString object the right-hand side of the operator
 *  - true if both strings are not equal.
 *  - false otherwise
 */
bool operator!=(ConstString const& lhs, ConstString const& rhs);
/**
 * Compares two constant string objects.
 * \param lhs ConstString object on the left-hand side of the operator
 * \param rhs ConstString object the right-hand side of the operator
 *  - true if lhs is less than rhs.
 *  - false otherwise
 */
bool operator<(ConstString const& lhs, ConstString const& rhs);
/**
 * Compares two constant string objects.
 * \param lhs ConstString object on the left-hand side of the operator
 * \param rhs ConstString object the right-hand side of the operator
 *  - true if lhs is greater than rhs.
 *  - false otherwise
 */
bool operator>(ConstString const& lhs, ConstString const& rhs);
/**
 * Compares two constant string objects.
 * \param lhs ConstString object on the left-hand side of the operator
 * \param rhs ConstString object the right-hand side of the operator
 *  - true if lhs is less than or equal to rhs.
 *  - false otherwise
 */
bool operator<=(ConstString const& lhs, ConstString const& rhs);
/**
 * Compares two constant string objects.
 * \param lhs ConstString object on the left-hand side of the operator
 * \param rhs ConstString object the right-hand side of the operator
 *  - true if lhs is greater than or equal to rhs.
 *  - false otherwise
 */
bool operator>=(ConstString const& lhs, ConstString const& rhs);

/* Implementation  */
inline ConstString::ConstString()
{
    _data   = nullptr;
    _length = 0U;
}

inline ConstString::ConstString(char const* const str, size_t const len)
{
    _data   = str;
    _length = (str != nullptr) ? len : 0U;
}

inline ConstString::ConstString(char const* const str)
{
    _data   = str;
    _length = (str != nullptr) ? strlen(str) : 0U;
}

inline ConstString::ConstString(ConstString const& other) = default;

inline ConstString& ConstString::operator=(ConstString const& other) = default;

inline bool ConstString::isEmpty() const { return _length == 0U; }

inline size_t ConstString::length() const { return _length; }

inline char ConstString::operator[](size_t const n) const
{
#ifndef ESR_NO_BUFFER_ASSERT
    estd_assert(n < _length);
#endif
    return _data[n];
}

inline int32_t ConstString::compare(ConstString const& other, bool const ignoreCase) const
{
    return ignoreCase ? compareIgnoreCase(other) : compare(other);
}

inline char const* ConstString::data() const { return _data; }

inline PlainSizedString const* ConstString::plain_str() const { return this; }

inline int32_t ConstString::toLower(int32_t const c)
{
    return ((c >= static_cast<int32_t>('A')) && (c <= static_cast<int32_t>('Z')))
               ? (c + static_cast<int32_t>('a') - static_cast<int32_t>('A'))
               : c;
}

inline bool operator==(ConstString const& lhs, ConstString const& rhs)
{
    return lhs.compare(rhs) == 0;
}

inline bool operator!=(ConstString const& lhs, ConstString const& rhs)
{
    return lhs.compare(rhs) != 0;
}

inline bool operator<(ConstString const& lhs, ConstString const& rhs)
{
    return lhs.compare(rhs) < 0;
}

inline bool operator>(ConstString const& lhs, ConstString const& rhs)
{
    return lhs.compare(rhs) > 0;
}

inline bool operator<=(ConstString const& lhs, ConstString const& rhs)
{
    return lhs.compare(rhs) <= 0;
}

inline bool operator>=(ConstString const& lhs, ConstString const& rhs)
{
    return lhs.compare(rhs) >= 0;
}

} /* namespace string */
} /* namespace util */
