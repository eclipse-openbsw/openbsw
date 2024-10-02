// Copyright 2024 Accenture.

/**
 * Contains estd::ordered_vector and estd::declare::ordered_vector.
 * \file
 * \ingroup estl_containers
 */
#ifndef GUARD_D090BE1B_6AD0_4794_AC78_DD9D7250C2D4
#define GUARD_D090BE1B_6AD0_4794_AC78_DD9D7250C2D4

#include "estd/algorithm.h"
#include "estd/assert.h"
#include "estd/vector.h"

#include <platform/estdint.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

namespace estd
{
/**
 * Implementation of a fixed-size ordered vector. Elements are inserted via the
 * insert method. The Comparator object defines the sort order.
 *
 * \tparam  T The type of elements stored in the ordered vector.
 * \tparam Comparator The sorting order.
 *
 * \section ordered_vector_memory_usage Memory Usage
 *
 * The memory usage in bytes for this ordered_vector will be:
 *
 * <b>32 bit OS</b>: N * sizeof(T) + 20
 * <b>64 bit OS</b>: N * sizeof(T) + 40
 *
 * \note It is possible there will be extra bytes used
 *       for proper memory alignment depending on your compiler settings.
 *
 * \section ordered_vector_example Usage example
 * \code{.cpp}
 * void insertRandom(estd::ordered_vector<int>& v)
 * {
 *     for(int i = 0; i < v.max_size(); ++i)
 *     {
 *         v.insert((rand() * 50);
 *     }
 *
 *     for(auto iter = v.begin(); iter != v.end(); ++iter)
 *     {
 *         printf("%d\n", *iter);
 *     }
 * }\endcode
 *
 * \see estd::declare::ordered_vector
 */

template<class T, class Comparator = std::less<T>>
class ordered_vector
{
public:
    using value_type             = T const;
    /** The typedef for this container */
    using this_type              = ordered_vector<T, Comparator>;
    /** An unsigned integral type */
    using size_type              = std::size_t;
    /** A const reference to the template parameter T */
    using const_reference        = T const&;
    /** A random access iterator for the sorted vector */
    using const_iterator         = T const*;
    /** A random access reverse iterator */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /**
     * A simple destructor. This destructor will not call the contained
     * object destructors. However, the derived class will call the
     * destructors on any objects in this vector.
     *
     * \see estd::declare::ordered_vector
     */
    ~ordered_vector() = default;

    /**
     * An assignment operator. Objects are copied from 'other',
     * replacing the objects in this vector.
     *
     * \param other The other ordered_vector to copy.
     * \return A reference to this.
     */
    this_type& operator=(this_type const& other);

    /**
     * Returns a const reference to the element at position 'n' in this vector.
     * Bounds checking is not performed.
     *
     * \param n The index into the vector.
     * \return A const reference to the requested value.
     */
    const_reference operator[](size_type n) const;

    /**
     * Returns a const reference to the element at position 'n' in this vector.
     * Bounds checking is performed.
     *
     * \param n The index into this vector.
     * \return A const reference to the requested value.
     * \assert{n < size()}
     */
    const_reference at(size_type n) const;

    /**
     * Adds the value to the sorted vector.
     *
     * \param value The value to insert into the vector.
     * \return An iterator to the newly inserted item.
     * \assert{!full()}
     */
    const_iterator insert(const_reference value);

    const_iterator find_or_insert(const_reference value);

    /**
     * Checks if the referenced value is in this vector
     *
     * \param value The value to search for.
     * \return
     * - true if the element exists
     * - false otherwise.
     */
    bool contains(const_reference value) const;

    /**
     * Removes all elements with a given value. Destructors are called
     * for any elements that are removed.
     *
     * \param value The value to find and remove.
     */
    void remove(const_reference value);

    /**
     * Removes all of the elements in the array. The destructors
     * are called for any object that has been inserted.
     */
    void clear();

    /**
     * Removes all elements fulfilling a given condition.
     * Destructors are called for any elements that are removed.
     *
     * \tparam  Predicate    Condition to check
     *
     * Predicate must implement:
     *
     * - bool operator()(T const& item)
     *
     * The function calls pred(*itr) for each element of the list, itr
     * being an iterator, and removes the element if pred(*itr) returns
     * true.
     */
    template<class Predicate>
    void remove_if(Predicate pred);

    /**
     * Erase element position. The destructor is called on the removed element.
     *
     * \param position An iterator pointing to the element to remove.
     * \return Returns an iterator to the next position
     */
    const_iterator erase(const_iterator position);

    /**
     * Erase elements [first, last). Destructors are called for each
     * element removed.
     *
     * \param first An iterator to the first element
     * \param last An iterator to the last element
     * \return An iterator to the last element.
     */
    const_iterator erase(const_iterator first, const_iterator last);

    /** Returns true if this vector is empty, false otherwise */
    bool empty() const;

    /** Returns true if this vector is full, false otherwise */
    bool full() const;

    /** The number of elements added to the sorted vector */
    size_type size() const;

    const_iterator data() const { return _values.data(); }

    void resize(size_t const n) { _values.resize(::estd::min(n, size())); }

    /** The max_size of this ordered_vector */
    size_type max_size() const;

    /*
     * const iterators
     */

    /** Returns a const iterator to the beginning of the vector */
    const_iterator begin() const;

    /** Returns a const iterator to the last element in the vector */
    const_iterator end() const;

    /**
     * Returns a reverse iterator pointing to the last element in the vector
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    const_reverse_iterator rbegin() const;

    /**
     * Returns a const reverse iterator pointing to the first element in the vector
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    const_reverse_iterator rend() const;

    /** Returns a const iterator to the beginning of the vector */
    const_iterator cbegin() const;

    /** Returns a const iterator to the last element in the vector */
    const_iterator cend() const;

    /**
     * Returns a const reverse iterator pointing to the last element in the vector
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    const_reverse_iterator crbegin() const;

    /**
     * Returns a const reverse iterator pointing to the first element in the vector
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    const_reverse_iterator crend() const;

    /**
     * Returns the internal comparator object.
     *
     * \return The internal value comparator object.
     */
    Comparator value_comp() const;

protected:
    /**
     * Constructor that accepts the underlying data structures
     * for storing the data.
     *
     * \param values A vector of values that will be used.
     * \param comp A comparison object.
     */
    ordered_vector(vector<T>& values, Comparator const& comp);

private:
    // private and not defined
    ordered_vector(this_type const& other) = delete;

    vector<T>& _values;
    Comparator _comp;
};

/**
 * Compares two ordered_vectors and returns true if they are identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the ordered_vectors are identical
 * - false otherwise.
 */
template<class T, class Comparator>
bool operator==(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs);

/**
 * Compares two ordered_vectors and returns true if they are not identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the ordered_vectors are not identical
 * - false otherwise.
 */
template<class T, class Comparator>
bool operator!=(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs);

/**
 * Compares two ordered_vectors and returns true if all elements
 * in the left-hand side are less than the elements in the
 * right hand side.
 *
 * Uses the Comparator object from the lhs instance.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is less than the right-hand side.
 * - false otherwise.
 */
template<class T, class Comparator>
bool operator<(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs);

/**
 * Compares two ordered_vectors and returns true if all elements
 * in the left-hand side are greater than the elements in the
 * right hand side.
 *
 * Uses the Comparator object from the lhs instance.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is greater than the right-hand side.
 * - false otherwise.
 */
template<class T, class Comparator>
bool operator>(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs);

/**
 * Compares two ordered_vectors and returns true if all elements
 * in the left-hand side are greater than or equal to the
 * elements in the right hand side.
 *
 * Uses the Comparator object from the lhs instance.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is greater than or equal to the right-hand side.
 * - false otherwise.
 */
template<class T, class Comparator>
bool operator>=(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs);

/**
 * Compares two ordered_vectors and returns true if all elements
 * in the left-hand side are less than or equal to the
 * elements in the right hand side.
 *
 * Uses the Comparator object from the lhs instance.
 *
 * \param lhs The left-hand side of the comparison.
 * \param rhs The right-hand side of the comparison.
 * \return
 * - true if the left-hand side is less than or equal to the right-hand side.
 * - false otherwise.
 */
template<class T, class Comparator>
bool operator<=(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs);

/*
 *
 * Class to declare a fixed-sized ordered_vector of size N
 *
 */
namespace declare
{
/**
 * Declaring a statically-sized ordered_vector object of size N.
 */
template<class T, std::size_t N, class Comparator = std::less<T>>
class ordered_vector : public ::estd::ordered_vector<T, Comparator>
{
public:
    /** A shortcut to the base class */
    using base      = ::estd::ordered_vector<T, Comparator>;
    /** A shortcut to this type */
    using this_type = ::estd::declare::ordered_vector<T, N, Comparator>;

    /** An unsigned integral type */
    using size_type              = typename base::size_type;
    /** A const reference to the template parameter T */
    using const_reference        = typename base::const_reference;
    /** A random access iterator for the sorted vector */
    using const_iterator         = typename base::const_iterator;
    /** A random access reverse iterator */
    using const_reverse_iterator = typename base::const_reverse_iterator;

    /**
     * Constructs an empty ordered_vector
     */
    explicit ordered_vector(Comparator const& comp = Comparator());

    /**
     * Copies the values of the other ordered_vector into this ordered_vector.
     */
    ordered_vector(base const& other);
    /**
     * Copies the values of the other ordered_vector into this ordered_vector.
     */
    ordered_vector(this_type const& other);

    /**
     * Calls the destructor on all contained objects.
     */
    ~ordered_vector();

    /**
     * Copies the values of the other ordered_vector into this ordered_vector.
     */
    this_type& operator=(base const& other);

    /**
     * Copies the values of the other ordered_vector into this ordered_vector.
     */
    this_type& operator=(this_type const& other);

private:
    ::estd::declare::vector<T, N> _values;
};

} // namespace declare

/*
 *
 * Implementation
 *
 */

/*
 *
 * Global functions
 *
 */

template<class T, class Comparator>
bool operator==(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template<class T, class Comparator>
bool operator!=(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs)
{
    return !(lhs == rhs);
}

template<class T, class Comparator>
bool operator<(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs)
{
    return std::lexicographical_compare(
        lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), lhs.value_comp());
}

template<class T, class Comparator>
bool operator>(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs)
{
    return (rhs < lhs);
}

template<class T, class Comparator>
bool operator>=(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs)
{
    return !(rhs > lhs);
}

template<class T, class Comparator>
bool operator<=(ordered_vector<T, Comparator> const& lhs, ordered_vector<T, Comparator> const& rhs)
{
    return !(lhs > rhs);
}

/*
 *
 * Member functions
 *
 */

template<class T, class Comparator>
inline ordered_vector<T, Comparator>::ordered_vector(vector<T>& values, Comparator const& comp)
: _values(values), _comp(comp)
{}

template<class T, class Comparator>
ordered_vector<T, Comparator>& ordered_vector<T, Comparator>::operator=(this_type const& other)
{
    _comp   = other._comp;
    _values = other._values;

    return *this;
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_reference
ordered_vector<T, Comparator>::operator[](size_type const n) const
{
    return _values[n];
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_reference
ordered_vector<T, Comparator>::at(size_type const n) const
{
    return _values.at(n);
}

template<class T, class Comparator>
typename ordered_vector<T, Comparator>::const_iterator
ordered_vector<T, Comparator>::insert(const_reference value)
{
    estd_assert(!full());
    const_iterator const position = std::lower_bound(cbegin(), cend(), value, _comp);
    (void)_values.insert(position, value);
    return position;
}

template<class T, class Comparator>
typename ordered_vector<T, Comparator>::const_iterator
ordered_vector<T, Comparator>::find_or_insert(const_reference value)
{
    const_iterator const position = std::lower_bound(cbegin(), cend(), value, _comp);
    if ((position == cend()) || _comp(value, *position))
    {
        if (full())
        {
            return end();
        }
        (void)_values.insert(position, value);
    }
    return position;
}

template<class T, class Comparator>
bool ordered_vector<T, Comparator>::contains(const_reference value) const
{
    const_iterator const l = std::lower_bound(cbegin(), cend(), value, _comp);
    return ((l != cend()) && (_comp(value, *l) == false));
}

template<class T, class Comparator>
void ordered_vector<T, Comparator>::remove(const_reference value)
{
    std::pair<const_iterator, const_iterator> const r
        = std::equal_range(cbegin(), cend(), value, _comp);
    (void)erase(r.first, r.second);
}

template<class T, class Comparator>
void ordered_vector<T, Comparator>::clear()
{
    _values.clear();
}

template<class T, class Comparator>
template<class Predicate>
void ordered_vector<T, Comparator>::remove_if(Predicate pred)
{
    const_iterator b = cbegin();
    // This is a safe way to remove elements from a container while iterating over the container

    while (b != cend())
    {
        if (pred(*b) == true)
        {
            b = _values.erase(b);
        }
        else
        {
            ++b;
        }
    }
}

template<class T, class Comparator>
typename ordered_vector<T, Comparator>::const_iterator
ordered_vector<T, Comparator>::erase(const_iterator const position)
{
    return _values.erase(position);
}

template<class T, class Comparator>
typename ordered_vector<T, Comparator>::const_iterator
ordered_vector<T, Comparator>::erase(const_iterator const first, const_iterator const last)
{
    return _values.erase(first, last);
}

template<class T, class Comparator>
inline bool ordered_vector<T, Comparator>::empty() const
{
    return _values.empty();
}

template<class T, class Comparator>
inline bool ordered_vector<T, Comparator>::full() const
{
    return _values.full();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::size_type ordered_vector<T, Comparator>::size() const
{
    return _values.size();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::size_type
ordered_vector<T, Comparator>::max_size() const
{
    return _values.max_size();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_iterator
ordered_vector<T, Comparator>::begin() const
{
    return _values.begin();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_iterator
ordered_vector<T, Comparator>::end() const
{
    return _values.end();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_reverse_iterator
ordered_vector<T, Comparator>::rbegin() const
{
    return _values.crbegin();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_reverse_iterator
ordered_vector<T, Comparator>::rend() const
{
    return _values.crend();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_iterator
ordered_vector<T, Comparator>::cbegin() const
{
    return _values.cbegin();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_iterator
ordered_vector<T, Comparator>::cend() const
{
    return _values.cend();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_reverse_iterator
ordered_vector<T, Comparator>::crbegin() const
{
    return _values.crbegin();
}

template<class T, class Comparator>
inline typename ordered_vector<T, Comparator>::const_reverse_iterator
ordered_vector<T, Comparator>::crend() const
{
    return _values.crend();
}

template<class T, class Comparator>
inline Comparator ordered_vector<T, Comparator>::value_comp() const
{
    return _comp;
}

/*
 * Implementation that takes a size_t N. Just used for constructing
 * a ordered_vector.
 */
namespace declare
{
template<class T, std::size_t N, class Comparator>
ordered_vector<T, N, Comparator>::ordered_vector(Comparator const& comp) : base(_values, comp)
{}

template<class T, std::size_t N, class Comparator>
ordered_vector<T, N, Comparator>::ordered_vector(base const& other) : base(_values, Comparator())
{
    base::operator=(other);
}

template<class T, std::size_t N, class Comparator>
ordered_vector<T, N, Comparator>::ordered_vector(this_type const& other)
: base(_values, Comparator())
{
    base::operator=(other);
}

template<class T, std::size_t N, class Comparator>
ordered_vector<T, N, Comparator>::~ordered_vector()
{
    base::clear();
}

template<class T, std::size_t N, class Comparator>
ordered_vector<T, N, Comparator>& ordered_vector<T, N, Comparator>::operator=(base const& other)
{
    base::operator=(other);
    return *this;
}

template<class T, std::size_t N, class Comparator>
ordered_vector<T, N, Comparator>&
ordered_vector<T, N, Comparator>::operator=(this_type const& other)
{
    base::operator=(other);
    return *this;
}

} // namespace declare
} // namespace estd

#endif // ESTD_SORTED_VECTOR_H_
