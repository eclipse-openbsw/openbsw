// Copyright 2024 Accenture.

#pragma once

#include "estd/assert.h"

#include <platform/estdint.h>

#include <algorithm>
#include <iterator>

namespace estd
{
/**
 * Helper struct to enable specialization for N == 0 without needing to duplicate the whole array
 * class.
 */
template<class T, size_t N>
struct array_data;

/**
 * Fixed-size sequence container that holds a specific number of elements ordered
 * in a strict linear sequence.
 *
 * [TPARAMS_START]
 * \tparam T    Type of elements of array, aliased by typedef value_type.
 * \tparam N    Size of the array provided as size_t.
 * [TPARAMS_END]
 */
template<class T, size_t N>
class array
{
public:
    // [PUBLIC_TYPES_START]
    /** Template parameter T */
    using value_type             = T;
    /** Unsigned integral type */
    using size_type              = std::size_t;
    /** Signed integral type */
    using difference_type        = std::ptrdiff_t;
    /** Reference to T */
    using reference              = T&;
    /** Const reference to T */
    using const_reference        = T const&;
    /** Pointer to T */
    using pointer                = T*;
    /** Pointer to const T */
    using const_pointer          = T const*;
    /** Random access iterator to value_type */
    using iterator               = pointer;
    /** Random access iterator to const value_type */
    using const_iterator         = const_pointer;
    /** Reverse iterator */
    using reverse_iterator       = std::reverse_iterator<iterator>;
    /** Constant reverse iterator */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    // [PUBLIC_TYPES_END]

    // [PUBLIC_API_START]
    /**
     * Access element at given position.
     *
     * \param n The index in the array.
     * \return A non-const reference to the requested object if 'n' is valid.
     *
     * \assert{n < size()}
     */
    reference at(size_type n);

    /**
     * Read access to element at given position.
     *
     * \param n The index in this array.
     * \return A const reference to the requested object if 'n' is valid.
     *
     * \assert{n < size()}
     */
    const_reference at(size_type n) const;

    /**
     * Unchecked access to element at given position.
     *
     * \param n The index in the array.
     * \return A non-const reference to the requested object.
     */
    reference operator[](size_type n);

    /**
     * Unchecked read only access to element at given position.
     *
     * \param n The index in the array.
     * \return A const reference to the requested object.
     */
    const_reference operator[](size_type n) const;

    /**
     * Access to first element.
     *
     * \return A non-const reference to the first object in the array.
     */
    reference front();

    /**
     * Read only access to first element.
     *
     * \return A const reference to the first object in the array.
     */
    const_reference front() const;

    /**
     * Access to last element.
     *
     * \return A non-const reference to the last object in the array.
     */
    reference back();

    /**
     * Read only access to last element.
     *
     * \return A const reference to the last object in the array.
     */
    const_reference back() const;

    /**
     * Returns pointer to internal data. Overload for non-const objects.
     *
     * \return A non-const pointer to the internal data. Use with care!
     */
    pointer data();

    /**
     * Returns const_pointer to internal data. Overload for const objects.
     *
     * \return A const pointer to the internal data. Use with care!
     */
    const_pointer data() const;

    /**
     * \return
     * - true if size() == 0
     * - false otherwise.
     */
    bool empty() const;

    /**
     * Returns the size of an array equals N passed to the constructor.
     */
    constexpr size_type size() const;

    /**
     * Return the max_size of an array equals size().
     */
    constexpr size_type max_size() const;

    /**
     * Fills the array with a given value.
     *
     * \param value The value to copy into each element of this array.
     */
    void fill(const_reference value);

    /**
     * Swaps the content of this array with a given other array by calling
     * size() times individual swap if both arrays have the same size.
     *
     * \param other The other array to swap.
     */
    void swap(array& other);

    /**
     * Returns iterator to beginning of this array.
     */
    iterator begin();

    /**
     * Returns iterator to end of this array.
     */
    iterator end();

    /**
     * Returns const_iterator to beginning of this array.
     */
    const_iterator begin() const;

    /**
     * Returns const_iterator to end of this array.
     */
    const_iterator end() const;

    /**
     * Returns a reverse iterator pointing to the last element in the array
     * (i.e., it's reverse beginning). Reverse iterators iterate backwards.
     *
     * \return A reverse iterator the reverse beginning of the array (end).
     */
    reverse_iterator rbegin();

    /**
     * Returns a reverse iterator pointing to the first element in the array
     * (i.e., it's reverse end). Reverse iterators iterate backwards.
     *
     * \return A reverse iterator to the reverse end (beginning).
     */
    reverse_iterator rend();

    /**
     * Returns a const reverse iterator pointing to the last element in the array
     * (i.e., it's reverse beginning). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator the reverse beginning of the array (end).
     */
    const_reverse_iterator rbegin() const;

    /**
     * Returns a const reverse iterator pointing to the first element in the array
     * (i.e., it's reverse end). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator to the reverse end (beginning).
     */
    const_reverse_iterator rend() const;

    /**
     * Returns const_iterator to beginning of this array.
     */
    const_iterator cbegin() const;

    /**
     * Returns const_iterator to end of this array.
     */
    const_iterator cend() const;

    /**
     * Returns a const reverse iterator pointing to the last element in the array
     * (i.e., it's reverse beginning). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator the reverse beginning of the array (end).
     */
    const_reverse_iterator crbegin() const;

    /**
     * Returns a const reverse iterator pointing to the first element in the array
     * (i.e., it's reverse end). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator to the reverse end (beginning).
     */
    const_reverse_iterator crend() const;
    // [PUBLIC_API_END]

    reference getReference(size_t const pos);

    const_reference getReference(size_t const pos) const;

    typename array_data<T, N>::type _values;
};

// function declarations

// [PUBLIC_FUNC_START]
/**
 * Compares two arrays and returns true if they are identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the arrays are identical
 * - false otherwise.
 */
template<class T, size_t N>
bool operator==(array<T, N> const& lhs, array<T, N> const& rhs);

/**
 * Compares two arrays and returns true if they are not identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the arrays are not identical
 * - false otherwise.
 */
template<class T, size_t N>
bool operator!=(array<T, N> const& lhs, array<T, N> const& rhs);

/**
 * Compares two arrays and returns true if all elements
 * in the left-hand side are less than the elements in the
 * right hand side.
 *
 * Requires T to have operator< defined.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is less than the right-hand side.
 * - false otherwise.
 */
template<class T, size_t N>
bool operator<(array<T, N> const& lhs, array<T, N> const& rhs);

/**
 * Compares two arrays and returns true if all elements
 * in the left-hand side are greater than the elements in the
 * right hand side.
 *
 * Requires T to have operator< defined.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is greater than the right-hand side.
 * - false otherwise.
 */
template<class T, size_t N>
bool operator>(array<T, N> const& lhs, array<T, N> const& rhs);

/**
 * Compares two arrays and returns true if all elements
 * in the left-hand side are greater than or equal to the
 * elements in the right hand side.
 *
 * Requires T to have operator< defined.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is greater than or equal to the right-hand side.
 * - false otherwise.
 */
template<class T, size_t N>
bool operator>=(array<T, N> const& lhs, array<T, N> const& rhs);

/**
 * Compares two arrays and returns true if all elements
 * in the left-hand side are less than or equal to the
 * elements in the right hand side.
 *
 * Requires T to have operator< defined.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is less than or equal to the right-hand side.
 * - false otherwise.
 */
template<class T, size_t N>
bool operator<=(array<T, N> const& lhs, array<T, N> const& rhs);

/**
 * Creates an array of one element from a given value.
 */
template<class T>
array<T, 1> make_array(T const& value);

// [PUBLIC_FUNC_END]

// function definitions

template<class T, size_t N>
typename array<T, N>::reference array<T, N>::at(size_type const n)
{
    estd_assert(n < N);
    return getReference(n);
}

template<class T, size_t N>
typename array<T, N>::const_reference array<T, N>::at(size_type const n) const
{
    estd_assert(n < N);
    return getReference(n);
}

template<class T, size_t N>
typename array<T, N>::reference array<T, N>::operator[](size_type const n)
{
    return getReference(n);
}

template<class T, size_t N>
typename array<T, N>::const_reference array<T, N>::operator[](size_type const n) const
{
    return getReference(n);
}

template<class T, size_t N>
typename array<T, N>::reference array<T, N>::front()
{
    return getReference(0U);
}

template<class T, size_t N>
typename array<T, N>::const_reference array<T, N>::front() const
{
    return getReference(0U);
}

template<class T, size_t N>
typename array<T, N>::reference array<T, N>::back()
{
    return getReference(N - 1U);
}

template<class T, size_t N>
typename array<T, N>::const_reference array<T, N>::back() const
{
    return getReference(N - 1U);
}

template<class T, size_t N>
typename array<T, N>::pointer array<T, N>::data()
{
    return _values;
}

template<class T, size_t N>
typename array<T, N>::const_pointer array<T, N>::data() const
{
    return _values;
}

template<class T, size_t N>
bool array<T, N>::empty() const
{
    return (0U == N);
}

template<class T, size_t N>
constexpr typename array<T, N>::size_type array<T, N>::size() const
{
    return N;
}

template<class T, size_t N>
constexpr typename array<T, N>::size_type array<T, N>::max_size() const
{
    return N;
}

template<class T, size_t N>
void array<T, N>::fill(const_reference value)
{
    // This call is ignored because from C++98 to C++11 the return value changed
    // from void to Iterator. To not get an MISRA error on newer compiler we ignore
    // it always.
    (void)::std::fill_n(array_data<T, N>::getPtr(*this, 0U), N, value);
}

template<class T, size_t N>
void array<T, N>::swap(array& other)
{
    std::swap(*this, other);
}

template<class T, size_t N>
typename array<T, N>::iterator array<T, N>::begin()
{
    return array_data<T, N>::getPtr(*this, 0U);
}

template<class T, size_t N>
typename array<T, N>::iterator array<T, N>::end()
{
    return array_data<T, N>::getPtr(*this, N);
}

template<class T, size_t N>
typename array<T, N>::const_iterator array<T, N>::begin() const
{
    return array_data<T, N>::getPtr(*this, 0U);
}

template<class T, size_t N>
typename array<T, N>::const_iterator array<T, N>::end() const
{
    return array_data<T, N>::getPtr(*this, N);
}

template<class T, size_t N>
typename array<T, N>::reverse_iterator array<T, N>::rbegin()
{
    return reverse_iterator(end());
}

template<class T, size_t N>
typename array<T, N>::reverse_iterator array<T, N>::rend()
{
    return reverse_iterator(begin());
}

template<class T, size_t N>
typename array<T, N>::const_reverse_iterator array<T, N>::rbegin() const
{
    return const_reverse_iterator(end());
}

template<class T, size_t N>
typename array<T, N>::const_reverse_iterator array<T, N>::rend() const
{
    return const_reverse_iterator(begin());
}

template<class T, size_t N>
typename array<T, N>::const_iterator array<T, N>::cbegin() const
{
    return begin();
}

template<class T, size_t N>
typename array<T, N>::const_iterator array<T, N>::cend() const
{
    return end();
}

template<class T, size_t N>
typename array<T, N>::const_reverse_iterator array<T, N>::crbegin() const
{
    return rbegin();
}

template<class T, size_t N>
typename array<T, N>::const_reverse_iterator array<T, N>::crend() const
{
    return rend();
}

template<class T, size_t N>
typename array<T, N>::reference array<T, N>::getReference(size_t const pos)
{
    return *array_data<T, N>::getPtr(*this, pos);
}

template<class T, size_t N>
typename array<T, N>::const_reference array<T, N>::getReference(size_t const pos) const
{
    return *array_data<T, N>::getPtr(*this, pos);
}

/**
 * Compares two arrays and returns true if they are identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the arrays are identical
 * - false otherwise.
 */
template<class T, size_t N>
bool operator==(array<T, N> const& lhs, array<T, N> const& rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/**
 * Compares two arrays and returns true if they are not identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the arrays are not identical
 * - false otherwise.
 */
template<class T, size_t N>
bool operator!=(array<T, N> const& lhs, array<T, N> const& rhs)
{
    return !(lhs == rhs);
}

/**
 * Compares two arrays and returns true if all elements
 * in the left-hand side are less than the elements in the
 * right hand side.
 *
 * Requires T to have operator< defined.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is less than the right-hand side.
 * - false otherwise.
 */
template<class T, size_t N>
bool operator<(array<T, N> const& lhs, array<T, N> const& rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/**
 * Compares two arrays and returns true if all elements
 * in the left-hand side are greater than the elements in the
 * right hand side.
 *
 * Requires T to have operator< defined.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is greater than the right-hand side.
 * - false otherwise.
 */
template<class T, size_t N>
bool operator>(array<T, N> const& lhs, array<T, N> const& rhs)
{
    return (rhs < lhs);
}

/**
 * Compares two arrays and returns true if all elements
 * in the left-hand side are greater than or equal to the
 * elements in the right hand side.
 *
 * Requires T to have operator< defined.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is greater than or equal to the right-hand side.
 * - false otherwise.
 */
template<class T, size_t N>
bool operator>=(array<T, N> const& lhs, array<T, N> const& rhs)
{
    return !(rhs > lhs);
}

/**
 * Compares two arrays and returns true if all elements
 * in the left-hand side are less than or equal to the
 * elements in the right hand side.
 *
 * Requires T to have operator< defined.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is less than or equal to the right-hand side.
 * - false otherwise.
 */
template<class T, size_t N>
bool operator<=(array<T, N> const& lhs, array<T, N> const& rhs)
{
    return !(lhs > rhs);
}

template<class T>
array<T, 1> make_array(T const& value)
{
    array<T, 1> const a = {{value}};
    return a;
}

template<class T, size_t N>
struct array_data
{
    using type = T[N];

    template<class A>
    static T* getPtr(A& a, size_t const n)
    {
        return &a._values[n];
    }

    template<class A>
    static T const* getPtr(A const& a, size_t const n)
    {
        return &a._values[n];
    }
};

template<class T>
struct array_data<T, 0>
{
    using type = T*;

    template<class A>
    static T* getPtr(A&, size_t const)
    {
        return nullptr;
    }

    template<class A>
    static T const* getPtr(A const&, size_t const)
    {
        return nullptr;
    }
};

} // namespace estd
