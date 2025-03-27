// Copyright 2024 Accenture.

/**
 * Contains estd::vector and estd::declare::vector.
 * \file
 * \ingroup estl_containers
 */
#pragma once

#include "estd/algorithm.h"
#include "estd/assert.h"
#include "estd/constructor.h"
#include "estd/type_traits.h"
#include "estd/type_utils.h"

#include <platform/estdint.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <type_traits>
#include <utility>

namespace estd
{
/**
 * A fixed-sized STL like vector.
 *
 * \tparam  T   Type of values of this vector.
 *
 * \section vector_memory_usage Memory Usage
 *
 * The memory usage in bytes for this vector will be:
 *
 * <b>32 bit OS</b>: N * sizeof(T) + 12\n
 * <b>64 bit OS</b>: N * sizeof(T) + 24
 *
 * \note It is possible there will be extra bytes used
 *       for proper memory alignment depending on your compiler settings.
 *
 * \section vector_example Usage example
 * \code{.cpp}
 * void addItemsToVector(estd::vector<int>& d)
 * {
 *     for(int i = 0; i < 10; ++i)
 *     {
 *         d.push_back(i);
 *     }
 * }
 *
 * void processVector(const estd::vector<int>& d)
 * {
 *     for(auto i = d.begin(); i ! = d.end(); ++i)
 *     {
 *         int item = *i;
 *         // do something with item
 *     }
 * }
 * \endcode
 *
 * This container supports adding objects of type T that do not have
 * a copy constructor or default constructor. Use the emplace methods
 * to create an instance of T directly into the vector.
 *
 * \section vector_emplace_example Emplace example
 * \code{.cpp}
 * void addEmplaceObject(estd::vector<ComplexObject>& d)
 * {
 *     // construct an instance of ComplexObject directly in the vector.
 *     // Calls the constructor: ComplexObject(1, 2, 3);
 *     d.emplace_back().construct(1, 2, 3);
 * }\endcode
 *
 * \see estd::declare::vector
 * \see estd::constructor
 */
template<class T>
class vector
{
public:
    /** The template parameter T */
    using value_type             = T;
    /** A reference to the template parameter T */
    using reference              = T&;
    /** A const reference to the template parameter T */
    using const_reference        = T const&;
    /** A pointer to the template parameter T */
    using pointer                = T*;
    /** A const pointer to the template parameter T */
    using const_pointer          = T const*;
    /** An unsigned integral type for the size */
    using size_type              = std::size_t;
    /** A signed integral type */
    using difference_type        = std::ptrdiff_t;
    /** A random access iterator */
    using iterator               = pointer;
    /** A const random access iterator */
    using const_iterator         = const_pointer;
    /** A random access reverse iterator */
    using reverse_iterator       = std::reverse_iterator<iterator>;
    /** A const random access reverse iterator */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    vector(vector<T> const& other) = delete;

    /**
     * A simple destructor. This destructor does not call the contained
     * object destructors. The derived class vector will call the destructors
     * on any objects in the vector.
     */
    ~vector() = default;

    /**
     * An assignment operator. Copies values from other to this. Replacing all
     * values in this vector.
     *
     * \param other The other vector to copy.
     * \return A reference to this.
     */
    vector<T>& operator=(vector<T> const& other);

    /**
     * Returns current size of this vector.
     *
     * \complexity Constant
     */
    size_type size() const;

    /**
     * Returns maximum number of elements this vector can hold.
     *
     * \complexity Constant
     */
    size_type max_size() const;

    /**
     *  Returns whether this vector is empty or not.
     *
     *  \return
     *  - true if size() == 0
     *  - false otherwise
     *  \complexity Constant
     */
    bool empty() const;

    /**
     *  Returns true if the vector is full.
     *
     *  \return
     *  - true if size() == max_size()
     *  - false otherwise
     *  \complexity Constant
     */
    bool full() const;

    /**
     * Returns a reference to the element at position 'n' in this vector.
     *
     * \param n An index into this vector.
     * \return A reference to the requested element.
     * \note
     * No bounds checking is done.
     */
    reference operator[](size_type n);

    /**
     * Returns a reference to the element at position 'n' in this vector.
     *
     * \param n An index into this vector.
     * \return A const reference to the requested element.
     * \note
     * No bounds checking is done.
     */
    const_reference operator[](size_type n) const;

    /**
     * Returns a reference to the element at position 'n' in this vector.
     *
     * \param n An index into this vector.
     * \return A reference to the requested element.
     * \assert{n < size()}
     */
    reference at(size_type n);

    /**
     * Returns a const reference to the element at position 'n' in this vector.
     *
     * \param n An index into this vector.
     * \return A const reference to the requested element.
     * \assert{n < size()}
     */
    const_reference at(size_type n) const;

    /**
     * Returns a reference to the first element of this vector.
     *
     * \return A reference to the first element in the vector.
     * \assert{!empty()}
     * \complexity  Constant.
     */
    reference front();

    /**
     * Returns a const reference to the first element of this vector.
     *
     * \return A const reference to the first element in the vector.
     * \assert{!empty()}
     * \complexity  Constant.
     */
    const_reference front() const;

    /**
     * Returns a reference to the last element of this vector.
     *
     * \return A reference to the last element
     * \assert{!empty()}
     * \complexity  Constant.
     */
    reference back();

    /**
     * Returns a const reference to the last element of this vector.
     *
     * \return A const reference to the last element
     * \assert{!empty()}
     * \complexity  Constant.
     */
    const_reference back() const;

    /**
     * Default constructs an element at the end of this vector and
     * returns a reference to it increasing this vector's size by one.
     *
     * \return A reference to the newly created object.
     * \assert{!full()}
     */
    reference push_back();

    /**
     * Copy constructs an element at the end of this vector from a
     * given value increasing this vector's size by one.
     *
     * \param value The value to copy into this vector.
     * \assert{!full()}
     */
    void push_back(const_reference value);

    /**
     * Returns a constructor object to the memory allocated at the end
     * of this vector increasing this vectors size by one.
     *
     * \return A constructor object that can be used to initialize an object.
     * \assert{!full()}
     */
    constructor<T> emplace_back();

    /**
     * Returns a constructor object to the memory allocated at the position
     * indicated by the 'position' iterator. Elements are shifted to make
     * room for the new object.
     *
     * \param position An iterator to where the new object should be constructed.
     * \return A constructor object that can be used to initialize an object.
     * \assert{!full()}
     */
    constructor<T> emplace(const_iterator position);

    /**
     *  Removes last element and destroys it.
     */
    void pop_back();

    /**
     *  Inserts given value into the vector before the specified iterator.
     */
    iterator insert(const_iterator position, const_reference value);

    /**
     *  Inserts n copies of given value into the vector before the specified iterator.
     */
    void insert(const_iterator position, size_type n, const_reference value);

    /**
     *  Inserts copies of the elements in the range [first,last) at position.
     */
    // this is inline due to a compiler problem not finding the method
    // when it is defined outside of the class.
    template<class InputIterator>
    typename std::enable_if<std::is_integral<InputIterator>::value == false, iterator>::type
    insert(const_iterator const position, InputIterator const first, InputIterator const last)
    {
        iterator const start = begin() + (position - cbegin());

        range_insert(position, first, last);

        return start;
    }

    /**
     *  Removes element at given position and destroys it.
     *  Returns iterator pointing to the next element or end().
     */
    iterator erase(const_iterator position);

    /**
     *  Removes a range [first, last) of elements destroying all of them.
     *  Returns iterator pointing to the next element or end().
     */
    iterator erase(const_iterator first, const_iterator last);

    /**
     *  Clears the vector. All elements will be destroyed.
     */
    void clear();

    /**
     * Replaces any element currently in this container and fills it with n
     * elements which all are initialized with a copy of value. Existing
     * elements are removed.
     *
     * \param n The number of elements to add
     * \param value The value to copy into this container.
     * \complexity  Linear in the number of elements added.
     *
     * \note
     * If n > max_size(), only max_size() elements will be added.
     */
    void assign(size_type n, const_reference value);

    /**
     * Replaces any element currently in this container and fills it with the
     * elements in [first, last).
     *
     * \param first The beginning iterator.
     * \param last The ending iterator.
     * \complexity  Linear in the number of elements added.
     *
     * \note
     * At most max_size() elements will be added.
     */
    template<class InputIterator>
    typename std::enable_if<std::is_integral<InputIterator>::value == false, void>::type
    assign(InputIterator first, InputIterator const last)
    {
        clear();
        while ((!full()) && (first != last))
        {
            const_reference item = *first;
            (void)emplace_back().construct(item);
            ++first;
        }
    }

    /**
     * Resizes the vector so that it contains n elements. If n is smaller
     * than the current size then any extra elements are destroyed. If n
     * is bigger than the current size then new elements are added. T
     * must have a default constructor and a copy constructor if elements
     * are added. If n is bigger than the max size of the container then
     * the function asserts.
     *
     * \param n The new size of the container.
     * \assert{n <= max_size()}
     */
    void resize(size_type n);

    /**
     * Resizes the vector so that it contains n elements. If n is smaller
     * than the current size then any extra elements are destroyed. If n
     * is bigger than the current size then new elements are added by
     * copying 'val'. T must have a copy constructor if elements
     * are added. If n is bigger than the max size of the container then
     * the function asserts.
     *
     * \param n The new size of the container.
     * \param val The new value to copy if the vector grows
     * \assert{n <= max_size()}
     */
    void resize(size_type n, value_type const& val);

    /** Returns a pointer the the data */
    pointer data();

    /** Returns a pointer the the data */
    const_pointer data() const;

    /** Returns an iterator to the beginning */
    iterator begin();

    /** Returns a const iterator to the beginning */
    const_iterator begin() const;

    /** Returns a const iterator to the beginning */
    const_iterator cbegin() const;

    /** Returns an iterator to the end */
    iterator end();

    /** Returns a const iterator to the end */
    const_iterator end() const;

    /** Returns a const iterator to the end */
    const_iterator cend() const;

    /**
     * Returns a reverse iterator pointing to the last element in the vector
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    reverse_iterator rbegin();

    /**
     * Returns a const reverse iterator pointing to the last element in the vector
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    const_reverse_iterator rbegin() const;

    /**
     * Returns a const reverse iterator pointing to the last element in the vector
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    const_reverse_iterator crbegin() const;

    /**
     * Returns a reverse iterator pointing to the first element in the vector
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    reverse_iterator rend();

    /**
     * Returns a const reverse iterator pointing to the first element in the vector
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    const_reverse_iterator rend() const;

    /**
     * Returns a const reverse iterator pointing to the first element in the vector
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    const_reverse_iterator crend() const;

protected:
    /**
     * Constructor to initialize this vector with the actual data to work with.
     * Used by the declare class.
     *
     * \param data The actual data for this vector
     * \param N The size of the data array
     */
    explicit vector(size_type const N) : _max_size(N) {}

private:
    size_type _max_size;
    size_type _size = 0U;

    template<class InputIterator>
    void range_insert(const_iterator position, InputIterator first, InputIterator last);
};

/**
 * Compares two vectors and returns true if they are identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the vectors are identical
 * - false otherwise.
 */
template<class T>
bool operator==(vector<T> const& lhs, vector<T> const& rhs);

/**
 * Compares two vectors and returns true if they are not identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the vectors are not identical
 * - false otherwise.
 */
template<class T>
bool operator!=(vector<T> const& lhs, vector<T> const& rhs);

/**
 * Compares two vectors and returns true if all elements
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
template<class T>
bool operator<(vector<T> const& lhs, vector<T> const& rhs);

/**
 * Compares two vectors and returns true if all elements
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
template<class T>
bool operator>(vector<T> const& lhs, vector<T> const& rhs);

/**
 * Compares two vectors and returns true if all elements
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
template<class T>
bool operator>=(vector<T> const& lhs, vector<T> const& rhs);

/**
 * Compares two vectors and returns true if all elements
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
template<class T>
bool operator<=(vector<T> const& lhs, vector<T> const& rhs);

/*
 * namespace to declare a vector
 */
namespace declare
{

/**
 * Helper struct for setting appropriate underlying data buffer type.
 * \tparam  T   Type of elements of the vector.
 * \tparam  N   Maximum number of values in the vector.
 */
template<class T, size_t N>
struct buf_type
{
    using type = aligned_mem<T>[N];
};

template<class T>
struct buf_type<T, 0>
{
    using type = struct
    {};
};

/**
 * STL like vector with static size.
 * \tparam  T   Type of values of this vector.
 * \tparam  N   Maximum number of values in this vector.
 */
template<class T, std::size_t N>
class vector : public ::estd::vector<T>
{
public:
    /** A shortcut to the base class */
    using base      = ::estd::vector<T>;
    /** A shortcut to this type */
    using this_type = ::estd::declare::vector<T, N>;

    /** The template parameter T */
    using value_type             = typename base::value_type;
    /** A reference to the template parameter T */
    using reference              = typename base::reference;
    /** A const reference to the template parameter T */
    using const_reference        = typename base::const_reference;
    /** A pointer to the template parameter T */
    using pointer                = typename base::pointer;
    /** A const pointer to the template parameter T */
    using const_pointer          = typename base::const_pointer;
    /** An unsigned integral type for the size */
    using size_type              = typename base::size_type;
    /** A signed integral type */
    using difference_type        = typename base::difference_type;
    /** A random access iterator */
    using iterator               = typename base::iterator;
    /** A const random access iterator */
    using const_iterator         = typename base::const_iterator;
    /** A random access reverse iterator */
    using reverse_iterator       = typename base::reverse_iterator;
    /** A const random access reverse iterator */
    using const_reverse_iterator = typename base::const_reverse_iterator;

    /**
     * Constructs an empty vector.
     */
    vector() : base(N), _realData() {}

    /**
     * Constructs a vector with n elements default constructing the values.
     */
    explicit vector(size_type n);

    /**
     * Constructs a vector with n elements initialized to a given value v.
     */
    vector(size_type n, const_reference value);

    /**
     * Copies the values of the other vector into this vector.
     */
    vector(base const& other);

    /**
     * Copies the values of the other vector into this vector.
     */
    vector(this_type const& other);

    /**
     * Calls the destructor on all contained objects.
     */
    ~vector();

    /**
     * Copies the values of the other vector into this vector.
     */
    this_type& operator=(base const& other);

    /**
     * Copies the values of the other vector into this vector.
     */
    this_type& operator=(this_type const& other);

private:
    typename buf_type<T, N>::type _realData;
};

} // namespace declare

template<class T>
inline bool operator==(vector<T> const& lhs, vector<T> const& rhs)
{
    return (lhs.size() == rhs.size()) && (std::equal(lhs.begin(), lhs.end(), rhs.begin()) == true);
}

template<class T>
inline bool operator!=(vector<T> const& lhs, vector<T> const& rhs)
{
    return !(lhs == rhs);
}

template<class T>
inline bool operator<(vector<T> const& lhs, vector<T> const& rhs)
{
    return (lhs.size() == rhs.size())
           && (std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end())
               == true);
}

template<class T>
inline bool operator>(vector<T> const& lhs, vector<T> const& rhs)
{
    return (rhs < lhs);
}

template<class T>
inline bool operator>=(vector<T> const& lhs, vector<T> const& rhs)
{
    return !(rhs > lhs);
}

template<class T>
inline bool operator<=(vector<T> const& lhs, vector<T> const& rhs)
{
    return !(lhs > rhs);
}

/*
 *
 * Implementation of vector
 *
 */

template<class T>
vector<T>& vector<T>::operator=(vector<T> const& other)
{
    // identical vector
    if (this == &other)
    {
        return *this;
    }

    estd_assert(_max_size >= other._size);

    if (size() >= other.size())
    {
        (void)std::copy(other.cbegin(), other.cend(), begin());
        (void)erase(begin() + other.size(), end());
    }
    else
    {
        (void)std::copy(other.cbegin(), other.cbegin() + size(), begin());
        (void)insert(begin() + size(), other.cbegin() + size(), other.cend());
    }

    return *this;
}

template<class T>
inline void vector<T>::assign(size_type n, const_reference value)
{
    clear();
    n = min(n, max_size());
    while (n > 0U)
    {
        (void)emplace_back().construct(value);
        --n;
    }
}

template<class T>
inline typename vector<T>::pointer vector<T>::data()
{
    return reinterpret_cast<T*>(after_me<declare::vector<T, 1>>(this));
}

template<class T>
inline typename vector<T>::const_pointer vector<T>::data() const
{
    return reinterpret_cast<T const*>(after_me<declare::vector<T, 1>>(this));
}

template<class T>
inline typename vector<T>::iterator vector<T>::begin()
{
    return type_utils<T>::cast_to_type(after_me<declare::vector<T, 1>>(this));
}

template<class T>
inline typename vector<T>::const_iterator vector<T>::begin() const
{
    return type_utils<T>::cast_const_to_type(after_me<declare::vector<T, 1>>(this));
}

template<class T>
inline typename vector<T>::const_iterator vector<T>::cbegin() const
{
    return begin();
}

template<class T>
inline typename vector<T>::iterator vector<T>::end()
{
    return type_utils<T>::cast_to_type(after_me<declare::vector<T, 1>>(this), _size);
}

template<class T>
inline typename vector<T>::const_iterator vector<T>::end() const
{
    return type_utils<T>::cast_const_to_type(after_me<declare::vector<T, 1>>(this), _size);
}

template<class T>
inline typename vector<T>::const_iterator vector<T>::cend() const
{
    return end();
}

template<class T>
inline typename vector<T>::reverse_iterator vector<T>::rbegin()
{
    return reverse_iterator(end());
}

template<class T>
inline typename vector<T>::const_reverse_iterator vector<T>::rbegin() const
{
    return const_reverse_iterator(end());
}

template<class T>
inline typename vector<T>::const_reverse_iterator vector<T>::crbegin() const
{
    return rbegin();
}

template<class T>
inline typename vector<T>::reverse_iterator vector<T>::rend()
{
    return reverse_iterator(begin());
}

template<class T>
inline typename vector<T>::const_reverse_iterator vector<T>::rend() const
{
    return const_reverse_iterator(begin());
}

template<class T>
inline typename vector<T>::const_reverse_iterator vector<T>::crend() const
{
    return rend();
}

template<class T>
inline typename vector<T>::size_type vector<T>::size() const
{
    return _size;
}

template<class T>
inline typename vector<T>::size_type vector<T>::max_size() const
{
    return _max_size;
}

template<class T>
inline bool vector<T>::empty() const
{
    return (0U == _size);
}

template<class T>
inline bool vector<T>::full() const
{
    return (_size == _max_size);
}

template<class T>
inline typename vector<T>::reference vector<T>::operator[](size_type const n)
{
    return *type_utils<T>::cast_to_type(after_me<declare::vector<T, 1>>(this), n);
}

template<class T>
inline typename vector<T>::const_reference vector<T>::operator[](size_type const n) const
{
    return *type_utils<T>::cast_const_to_type(after_me<declare::vector<T, 1>>(this), n);
}

template<class T>
inline typename vector<T>::reference vector<T>::at(size_type const n)
{
    estd_assert(n < size());
    return operator[](n);
}

template<class T>
inline typename vector<T>::const_reference vector<T>::at(size_type const n) const
{
    estd_assert(n < size());
    return operator[](n);
}

template<class T>
inline typename vector<T>::reference vector<T>::front()
{
    estd_assert(size() > 0U);
    return *type_utils<T>::cast_to_type(after_me<declare::vector<T, 1>>(this));
}

template<class T>
inline typename vector<T>::const_reference vector<T>::front() const
{
    estd_assert(size() > 0U);
    return *type_utils<T>::cast_const_to_type(after_me<declare::vector<T, 1>>(this));
}

template<class T>
inline typename vector<T>::reference vector<T>::back()
{
    estd_assert(size() > 0U);
    size_type const pos = _size - 1U;
    return *type_utils<T>::cast_to_type(after_me<declare::vector<T, 1>>(this), pos);
}

template<class T>
inline typename vector<T>::const_reference vector<T>::back() const
{
    estd_assert(size() > 0U);
    size_type const pos = _size - 1U;
    return *type_utils<T>::cast_const_to_type(after_me<declare::vector<T, 1>>(this), pos);
}

template<class T>
inline typename vector<T>::reference vector<T>::push_back()
{
    estd_assert(!full());
    reference result = constructor<T>(after_me<declare::vector<T, 1>>(this), _size).construct();
    _size++;
    return result;
}

template<class T>
inline void vector<T>::push_back(const_reference value)
{
    estd_assert(!full());
    (void)constructor<T>(after_me<declare::vector<T, 1>>(this), _size).construct(value);
    _size++;
}

template<class T>
inline constructor<T> vector<T>::emplace_back()
{
    estd_assert(!full());
    auto const result = constructor<T>(after_me<declare::vector<T, 1>>(this), _size);
    _size++;
    return result;
}

template<class T>
inline constructor<T> vector<T>::emplace(const_iterator const position)
{
    estd_assert(!full());
    iterator const first = begin() + (position - cbegin());
    if (first < end())
    {
        // default construct new last element
        (void)constructor<T>(after_me<declare::vector<T, 1>>(this), _size).construct();
        // move all existing elements to the right by one
        (void)std::move_backward(first, end(), end() + 1U);
    }
    ++_size;
    return constructor<T>(type_utils<T>::cast_to_raw(first));
}

template<class T>
inline void vector<T>::pop_back()
{
    estd_assert(size() > 0U);
    type_utils<T>::destroy(after_me<declare::vector<T, 1>>(this), _size - 1);
    _size--;
}

template<class T>
inline typename vector<T>::iterator
vector<T>::insert(const_iterator const position, const_reference value)
{
    insert(position, 1U, value);
    return begin() + (position - cbegin());
}

template<class T>
inline void
vector<T>::insert(const_iterator const position, size_type const n, const_reference value)
{
    estd_assert((size() + n) <= max_size());
    iterator insertItr   = begin() + (position - cbegin());
    auto const diffToEnd = static_cast<difference_type>(cend() - position);
    auto const numToMove = static_cast<size_type>(diffToEnd);

    if (n > numToMove) // appended elements will consist of moved ones and new copies of 'value'
    {
        // append new copies of 'value'
        for (size_t i = 0; i < n - numToMove; ++i)
        {
            (void)emplace_back().construct(value);
        }
        // append new elements that are copies of existing ones
        iterator srcItr = insertItr;
        for (size_t i = n - numToMove; i < n; ++i)
        {
            (void)emplace_back().construct(::std::move(*srcItr));
            ++srcItr;
        }
        // create remaining new copies of 'value' at the given position
        for (size_t i = 0; i < numToMove; ++i)
        {
            *insertItr = value;
            ++insertItr;
        }
    }
    else // appended elements will consist of moved ones only
    {
        // append new elements that are copies of existing elements
        iterator srcItr = end() - n;
        for (size_t i = 0; i < n; ++i)
        {
            (void)emplace_back().construct(::std::move(*srcItr));
            ++srcItr;
        }
        // make more room by moving existing elements in reverse order
        srcItr          = end() - 2U * n - 1;
        iterator dstItr = end() - n - 1;
        for (size_t i = 0; i < numToMove - n; ++i)
        {
            *dstItr = ::std::move(*srcItr);
            --dstItr;
            --srcItr;
        }
        // create new copies of 'value' at the given position
        for (size_t i = 0; i < n; ++i)
        {
            *insertItr = value;
            ++insertItr;
        }
    }
}

template<class T>
inline typename vector<T>::iterator vector<T>::erase(const_iterator const position)
{
    if (position == end())
    {
        // Strictly speaking, this is undefined behaviour as 'position' must be
        // 'valid and dereferenceable' but 'end()' is not dereferenceable.
        return end();
    }
    return erase(position, position + 1);
}

template<class T>
typename vector<T>::iterator vector<T>::erase(const_iterator const first, const_iterator const last)
{
    estd_assert(last <= cend());
    if (last < first) // should this be an assert?
    {
        return begin() + (last - cbegin());
    }
    // move all elements to front
    iterator const srcIter    = begin() + (last - cbegin());
    iterator dstItr           = begin() + (first - cbegin());
    iterator const returnItem = dstItr;
    dstItr                    = ::std::move(srcIter, end(), dstItr);

    // destroy empty (after move) elements
    while (dstItr != end())
    {
        (*dstItr).~T();
        ++dstItr;
    }

    difference_type const numErased = last - first;
    _size -= static_cast<size_type>(numErased);
    return returnItem;
}

template<class T>
void vector<T>::clear()
{
    for (size_t i = 0U; i < _size; ++i)
    {
        type_utils<T>::destroy(after_me<declare::vector<T, 1>>(this), i);
    }
    _size = 0U;
}

template<class T>
void vector<T>::resize(size_type const n)
{
    estd_assert(n <= max_size());

    auto const currentSize = size();

    for (size_type i = n; i < currentSize; ++i)
    {
        type_utils<T>::destroy(after_me<declare::vector<T, 1>>(this), i);
    }

    for (size_type i = currentSize; i < n; ++i)
    {
        (void)constructor<T>(after_me<declare::vector<T, 1>>(this), i).construct();
    }
    _size = n;
}

template<class T>
void vector<T>::resize(size_type const n, value_type const& val)
{
    estd_assert(n <= max_size());

    auto const currentSize = size();

    for (size_type i = n; i < currentSize; ++i)
    {
        type_utils<T>::destroy(after_me<declare::vector<T, 1>>(this), i);
    }

    for (size_type i = currentSize; i < n; ++i)
    {
        (void)constructor<T>(after_me<declare::vector<T, 1>>(this), i).construct(val);
    }

    _size = n;
}

template<class T>
template<class InputIterator>
void vector<T>::range_insert(const_iterator position, InputIterator first, InputIterator const last)
{
    while ((!full()) && (first != last))
    {
        T const& item = *first;
        (void)vector::insert(position, item);
        ++position;
        ++first;
    }
}

namespace declare
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4351)
#endif
template<class T, std::size_t N>
vector<T, N>::vector(size_type const n, const_reference value) : ::estd::vector<T>(N), _realData()
{
    base::assign(::estd::min(n, N), value);
}

template<class T, std::size_t N>
vector<T, N>::vector(size_type const n) : ::estd::vector<T>(N), _realData()
{
    for (size_t i = 0U; i < n; ++i)
    {
        (void)base::emplace_back().construct();
    }
}

// Intended non explicit conversion constructor
template<class T, std::size_t N>
vector<T, N>::vector(base const& other) : ::estd::vector<T>(N), _realData()
{
    base::operator=(other);
}

template<class T, std::size_t N>
vector<T, N>::vector(this_type const& other) : ::estd::vector<T>(N), _realData()
{
    base::operator=(other);
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

template<class T, std::size_t N>
vector<T, N>::~vector()
{
    base::clear();
}

template<class T, std::size_t N>
vector<T, N>& vector<T, N>::operator=(base const& other)
{
    base::operator=(other);
    return *this;
}

template<class T, std::size_t N>
vector<T, N>& vector<T, N>::operator=(this_type const& other)
{
    base::operator=(other);
    return *this;
}

} // namespace declare
} // namespace estd
