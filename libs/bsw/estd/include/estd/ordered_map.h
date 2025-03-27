// Copyright 2024 Accenture.

/**
 * Contains estd::ordered_map and estd::declare::ordered_map.
 * \file
 * \ingroup estl_containers
 */
#pragma once

#include "estd/assert.h"
#include "estd/constructor.h"
#include "estd/vector.h"

#include <platform/estdint.h>

#include <functional>
#include <utility>

namespace estd
{
/**
 * A C++ std::map like class implemented using a sorted vector.
 *
 * \section keys
 * Two keys are considered equivalent if Compare returns false reflexively.
 *
 * \tparam  Key Key type of elements in this map.
 * \tparam  T Type of data elements mapped to a key.
 * \tparam  Compare Compare function object for type Key.
 *
 * \section ordered_map_memory_usage Memory Usage
 *
 * The memory usage in bytes for this ordered_map will be:
 *
 * <b>32 bit OS</b>: N * (sizeof(Key) + sizeof(T)) + 20\n
 * <b>64 bit OS</b>: N * (sizeof(Key) + sizeof(T)) + 40
 *
 * \note It is possible there will be extra bytes used
 *       for proper memory alignment depending on your compiler settings.
 *
 *
 */
template<class Key, class T, class Compare = std::less<Key>>
class ordered_map
{
public:
    /** The template parameter Key */
    using key_type        = Key;
    /** The template parameter value */
    using mapped_type     = T;
    /** A pair of Key and Value */
    using value_type      = std::pair<Key const, T>;
    /** A function object to compare Key's */
    using key_compare     = Compare;
    /** A reference to a Key and Value */
    using reference       = value_type&;
    /** A const reference to a Key and Value */
    using const_reference = value_type const&;
    /** A pointer to a Key and Value */
    using pointer         = value_type*;
    /** A const pointer to a Key and Value */
    using const_pointer   = value_type const*;
    /** An unsigned integral type for sizes */
    using size_type       = std::size_t;
    /** A signed integral type */
    using difference_type = std::ptrdiff_t;

    /** A utility class predicate for comparing key and values */
    class value_compare
    {
    private:
        using container_type = ::estd::ordered_map<Key, T, Compare>;

        Compare _valueComp;

    public:
        /** Creates a new value_compare class with the specified Compare object */
        explicit value_compare(Compare c);
        /** The return type of the predicate */
        using result_type          = bool;
        /** The first parameter type */
        using first_argument_type  = container_type::value_type;
        /** The second parameter type */
        using second_argument_type = container_type::value_type;

        /** Function overload, returns the result of the Compare instance */
        bool operator()(first_argument_type const& x, second_argument_type const& y) const;
    };

private:
    using data_type = vector<value_type>;

public:
    /** A random access iterator */
    using iterator               = typename data_type::iterator;
    /** A const random access iterator */
    using const_iterator         = typename data_type::const_iterator;
    /** A reverse random access iterator */
    using reverse_iterator       = typename data_type::reverse_iterator;
    /** A const reverse random access iterator */
    using const_reverse_iterator = typename data_type::const_reverse_iterator;

    ordered_map(ordered_map const& other) = delete;

    /**
     * A simple destructor. This destructor does not call the contained
     * object destructors. The derived declare class ordered_map will call the
     * destructors.
     *
     * \see estd::declare::ordered_map
     */
    ~ordered_map() = default;

    /**
     * An assignment operator. Copies the elements from other into
     * this ordered_map.
     *
     * \param other The other ordered map to copy.
     * \return A reference to this.
     */
    ordered_map& operator=(ordered_map const& other);

    /**
     * Returns whether this container is empty or not.
     */
    bool empty() const;

    /**
     * Returns whether this container is full or not.
     */
    bool full() const;

    /**
     * Returns the current number of entries in this map.
     */
    size_type size() const;

    /**
     * Returns the maximum number of entries this map can hold.
     */
    size_type max_size() const;

    /**
     * Removes all elements from this ordered_map, destroying them and leaving this ordered_map's
     * size at 0.
     */
    void clear();

    /**
     * Counts the number of elements with a key equal to a given key.
     * \return  As all elements in this map are unique
     * - 0 if none was found
     * - 1 if one was found
     */
    size_type count(key_type const& key) const;

    /**
     * Copies a given value to this map increasing the size of this
     * container by one if the value did not exist in the map before.
     *
     * \return  Pair of iterator and bool
     * - <iterator, true> if the element did not exist before, iterator to
     * the inserted element.
     * - <iterator, false> if the element did exist before, iterator to
     * the previously inserted element.
     */
    std::pair<iterator, bool> insert(const_reference value);

    /**
     * Adds a new key to the map if the key does not already exist. The
     * value is a default constructed T() object.
     *
     * \return  Pair of iterator and bool
     * - <iterator, true> if the element did not exist before, iterator to
     * the inserted element.
     * - <iterator, false> if the element did exist before, iterator to
     * the previously inserted element.
     */
    std::pair<iterator, bool> emplace(key_type const& key);

    /**
     * Removes an element at a given position destroying it reducing the size of
     * this ordered_map by one.
     * \return  Iterator pointing to the position immediately following the element erased.
     */
    iterator erase(const_iterator position);

    /**
     * Removes elements with a given key k destroying them reducing the size of this
     * ordered_map by one if an element with k existed.
     * \return  Number of elements removed.
     */
    size_type erase(key_type const& key);

    /**
     * Removes all elements within a given range [first, last), i.e. all elements including first
     * but excluding last. All removed elements will be destroyed and the size of this ordered_map
     * will be decreased by the number of elements removed. \return  Iterator pointing to the
     * position immediately following the last of the elements erased.
     */
    iterator erase(const_iterator first, const_iterator last);

    /**
     * If k matches the key of an element in the container, returns a
     * reference to its mapped value.
     * If k does not match any element, inserts a new one with key k and
     * returns a reference to it.
     */
    mapped_type& operator[](key_type const& key);

    /**
     * Returns a reference to the mapped value of the element identified
     * with 'key'.
     *
     * \assert{if key does not match any element in the container.}
     */
    mapped_type& at(key_type const& key);

    /**
     * Returns a const reference to the mapped value of the element identified
     * with 'key'.
     *
     * \assert{if key does not match any element in the container.}
     */
    mapped_type const& at(key_type const& key) const;

    /**
     * Searches this map for an element with a key equivalent to key and
     * returns an iterator to it if found, otherwise it returns end().
     */
    iterator find(key_type const& key);

    /**
     * Searches this map for an element with a key equivalent to key and
     * returns a const iterator to it if found, otherwise it returns end().
     */
    const_iterator find(key_type const& key) const;

    /**
     * Returns an iterator pointing to the first element in this map
     * whose key is not considered to go before 'key' (i.e., either it is
     * equivalent or goes after)
     */
    iterator lower_bound(key_type const& key);

    /**
     * Returns a const iterator pointing to the first element in this map
     * whose key is not considered to go before 'key' (i.e., either it is
     * equivalent or goes after)
     */
    const_iterator lower_bound(key_type const& key) const;

    /**
     * Returns an iterator pointing to the first element in this map
     * whose key is considered to go after 'key'.
     */
    iterator upper_bound(key_type const& key);

    /**
     * Returns a const iterator pointing to the first element in this map
     * whose key is considered to go after 'key'.
     */
    const_iterator upper_bound(key_type const& key) const;

    /**
     * Returns the bounds of a range that includes all the elements in
     * this map which have a key equivalent to key.
     */
    std::pair<iterator, iterator> equal_range(key_type const& key);

    /**
     * Returns the bounds of a range that includes all the elements in
     * this map which have a key equivalent to key.
     */
    std::pair<const_iterator, const_iterator> equal_range(key_type const& key) const;

    /**
     * Returns a copy of the key_compare object used by this map.
     */
    key_compare key_comp() const;

    /**
     * Returns a comparison object that can be used to compare two
     * elements to get whether the key of the first one goes before
     * the second. The arguments taken by this binary function object
     * are of type value_type.
     */
    value_compare value_comp() const;

    /** Returns an iterator to the beginning of the map */
    iterator begin();

    /** Returns a const iterator to the beginning of the map */
    const_iterator begin() const;

    /** Returns a const iterator to the beginning of the map */
    const_iterator cbegin() const;

    /** Returns an iterator to the end of the map */
    iterator end();

    /** Returns a const iterator to the end of the map */
    const_iterator end() const;

    /** Returns a const iterator to the end of the map */
    const_iterator cend() const;

    /**
     * Returns a reverse iterator pointing to the last element in the map
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    reverse_iterator rbegin();

    /**
     * Returns a reverse iterator pointing to the first element in the map
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    reverse_iterator rend();

    /**
     * Returns a const reverse iterator pointing to the last element in the map
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    const_reverse_iterator rbegin() const;

    /**
     * Returns a const reverse iterator pointing to the first element in the map
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    const_reverse_iterator rend() const;

    /**
     * Returns a const reverse iterator pointing to the last element in the map
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    const_reverse_iterator crbegin() const;

    /**
     * Returns a const reverse iterator pointing to the first element in the map
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    const_reverse_iterator crend() const;

protected:
    /**
     * Constructor to initialize this ordered_map with the actual data necessary.
     * Used by the declare class.
     *
     * \param data The actual data for this ordered_map
     * \param comp The comparison object for comparing keys
     *
     * \see estd::declare::ordered_map
     */
    ordered_map(data_type& data, key_compare const& comp);

private:
    size_type lower_bound_search(key_type const& key) const;
    size_type upper_bound_search(key_type const& key) const;
    value_type& emplace_uninitialized(iterator pos);
    bool key_eq(key_type const& x, key_type const& y) const;

    data_type& _values;
    key_compare _comp;
};

/**
 * Compares two ordered_maps and returns true if they are identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the ordered_maps are identical
 * - false otherwise.
 */
template<class Key, class T, class Compare>
bool operator==(ordered_map<Key, T, Compare> const& lhs, ordered_map<Key, T, Compare> const& rhs);
/**
 * Compares two ordered_maps and returns true if they are not identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the ordered_maps are not identical
 * - false otherwise.
 */
template<class Key, class T, class Compare>
bool operator!=(ordered_map<Key, T, Compare> const& lhs, ordered_map<Key, T, Compare> const& rhs);

namespace declare
{
/**
 * Declaring a statically-sized ordered_map object of size N.
 */
template<class Key, class T, std::size_t N, class Compare = std::less<Key>>
class ordered_map : public ::estd::ordered_map<Key, T, Compare>
{
public:
    /** A shortcut to the base class */
    using base      = ::estd::ordered_map<Key, T, Compare>;
    /** A shortcut to this type */
    using this_type = ::estd::declare::ordered_map<Key, T, N, Compare>;

    /** The template parameter Key */
    using key_type               = typename base::key_type;
    /** The template parameter value */
    using mapped_type            = typename base::mapped_type;
    /** A pair of Key and Value */
    using value_type             = typename base::value_type;
    /** A function object to compare Key's */
    using key_compare            = typename base::key_compare;
    /** A utility class predicate for comparing key and values */
    using value_compare          = typename base::value_compare;
    /** A reference to a Key and Value */
    using reference              = typename base::reference;
    /** A const reference to a Key and Value */
    using const_reference        = typename base::const_reference;
    /** A pointer to a Key and Value */
    using pointer                = typename base::pointer;
    /** A const pointer to a Key and Value */
    using const_pointer          = typename base::const_pointer;
    /** An unsigned integral type for sizes */
    using size_type              = typename base::size_type;
    /** A signed integral type */
    using difference_type        = typename base::difference_type;
    /** A random access iterator */
    using iterator               = typename base::iterator;
    /** A const random access iterator */
    using const_iterator         = typename base::const_iterator;
    /** A reverse random access iterator */
    using reverse_iterator       = typename base::reverse_iterator;
    /** A const reverse random access iterator */
    using const_reverse_iterator = typename base::const_reverse_iterator;

    /**
     * Constructs an empty ordered_map.
     */
    explicit ordered_map(key_compare const& comp = key_compare());

    /**
     * Copies the values of the other ordered_map into this ordered_map.
     */
    ordered_map(base const& other);

    /**
     * Copies the values of the other ordered_map into this ordered_map.
     */
    ordered_map(this_type const& other);

    /**
     * Calls the destructor on all contained objects.
     */
    ~ordered_map();

    /**
     * Copies the values of the other ordered_map into this ordered_map.
     */
    this_type& operator=(base const& other);

    /**
     * Copies the values of the other ordered_map into this ordered_map.
     */
    this_type& operator=(this_type const& other);

private:
    ::estd::declare::vector<value_type, N> _realValues;
};

} // namespace declare

/*
 *
 * Implementation
 *
 */

template<class Key, class T, class Compare>
inline bool
operator==(ordered_map<Key, T, Compare> const& lhs, ordered_map<Key, T, Compare> const& rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }
    for (typename ordered_map<Key, T, Compare>::const_iterator itr = lhs.cbegin();
         itr != lhs.cend();
         ++itr)
    {
        if (rhs.count(itr->first) > 0U)
        {
            if (!(rhs.at(itr->first) == itr->second))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

template<class Key, class T, class Compare>
inline bool
operator!=(ordered_map<Key, T, Compare> const& lhs, ordered_map<Key, T, Compare> const& rhs)
{
    return !(lhs == rhs);
}

/*
 *
 * class ordered_map
 *
 */
// protected
template<class Key, class T, class Compare>
ordered_map<Key, T, Compare>&
ordered_map<Key, T, Compare>::operator=(ordered_map<Key, T, Compare> const& other)
{
    if (&_values == &other._values)
    {
        return *this;
    }

    clear();

    for (const_iterator iter = other.begin(); iter != other.end(); ++iter)
    {
        (void)_values.emplace_back().construct(*iter);
    }

    return *this;
}

template<class Key, class T, class Compare>
inline ordered_map<Key, T, Compare>::ordered_map(data_type& data, key_compare const& comp)
: _values(data), _comp(comp)
{}

template<class Key, class T, class Compare>
inline bool ordered_map<Key, T, Compare>::empty() const
{
    return _values.empty();
}

template<class Key, class T, class Compare>
inline bool ordered_map<Key, T, Compare>::full() const
{
    return _values.full();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::size_type ordered_map<Key, T, Compare>::size() const
{
    return _values.size();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::size_type
ordered_map<Key, T, Compare>::max_size() const
{
    return _values.max_size();
}

template<class Key, class T, class Compare>
inline void ordered_map<Key, T, Compare>::clear()
{
    _values.clear();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::size_type
ordered_map<Key, T, Compare>::count(key_type const& key) const
{
    if (empty())
    {
        return 0U;
    }
    size_type const i = lower_bound_search(key);
    if (i >= max_size())
    {
        return 0U;
    }

    if (key_eq(_values[i].first, key))
    {
        return 1U;
    }

    return 0U;
}

template<class Key, class T, class Compare>
std::pair<typename ordered_map<Key, T, Compare>::iterator, bool>
ordered_map<Key, T, Compare>::insert(const_reference value)
{
    iterator const pos = lower_bound(value.first);
    if ((pos != end()) && key_eq(pos->first, value.first))
    {
        return std::make_pair(pos, false);
    }

    auto& v = emplace_uninitialized(pos);

    // Constructor needs to be able to modify the memory for placement new.
    // But v.first is a const object so we cast away the const.
    (void)constructor<key_type const>(type_utils<key_type const>::const_cast_to_void(&(v.first)))
        .construct(value.first);
    (void)constructor<mapped_type>(&v.second).construct(value.second);

    return std::make_pair(pos, true);
}

template<class Key, class T, class Compare>
std::pair<typename ordered_map<Key, T, Compare>::iterator, bool>
ordered_map<Key, T, Compare>::emplace(key_type const& key)
{
    iterator const pos = lower_bound(key);
    if ((pos != end()) && key_eq(pos->first, key))
    {
        return std::make_pair(pos, false);
    }

    auto& v = emplace_uninitialized(pos);

    // Constructor needs to be able to modify the memory for placement new.
    // But v.first is a const object so we cast away the const.
    (void)constructor<key_type const>(type_utils<key_type const>::const_cast_to_void(&(v.first)))
        .construct(key);
    (void)constructor<mapped_type>(&v.second).construct();

    return std::make_pair(pos, true);
}

template<class Key, class T, class Compare>
typename ordered_map<Key, T, Compare>::iterator
ordered_map<Key, T, Compare>::erase(const_iterator const position)
{
    if (position == end())
    {
        // Strictly speaking, this is undefined behaviour as 'position' must be
        // 'valid and dereferenceable' but 'end()' is not dereferenceable.
        return end();
    }
    return erase(position, position + 1U);
}

template<class Key, class T, class Compare>
typename ordered_map<Key, T, Compare>::size_type
ordered_map<Key, T, Compare>::erase(key_type const& key)
{
    const_iterator const ci = find(key);
    if (ci == cend())
    {
        return 0U;
    }

    (void)erase(ci);
    return 1U;
}

template<class Key, class T, class Compare>
typename ordered_map<Key, T, Compare>::iterator
ordered_map<Key, T, Compare>::erase(const_iterator const first, const_iterator const last)
{
    estd_assert(last <= cend());
    if (last <= first)
    {
        return begin() + (last - cbegin()); // empty range
    }

    // copy elements after range to front of range
    iterator const returnItem = begin() + (first - cbegin());
    iterator dstIter          = returnItem;
    iterator srcIter          = begin() + (last - cbegin());
    while (srcIter != end())
    {
        dstIter->~value_type();
        (void)constructor<value_type>(type_utils<value_type>::cast_to_raw(dstIter))
            .construct(::std::move(*srcIter));
        ++dstIter;
        ++srcIter;
    }

    // destroy excess tail elements
    auto const numToPop = static_cast<size_type>(static_cast<difference_type>(last - first));
    for (size_type i = 0; i < numToPop; ++i)
    {
        _values.pop_back();
    }
    return returnItem;
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::mapped_type&
ordered_map<Key, T, Compare>::operator[](key_type const& key)
{
    iterator const pos = lower_bound(key);
    if ((pos != end()) && key_eq(pos->first, key))
    {
        return pos->second;
    }

    ordered_map::value_type& v = emplace_uninitialized(pos);

    // Constructor needs to be able to modify the memory for placement new.
    // But v.first is a const object so we cast away the const.
    (void)constructor<key_type const>(type_utils<key_type const>::const_cast_to_void(&(v.first)))
        .construct(key);
    (void)constructor<mapped_type>(&v.second).construct();

    return v.second;
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::mapped_type&
ordered_map<Key, T, Compare>::at(key_type const& key)
{
    iterator const pos = lower_bound(key);
    estd_assert((pos != end()) && key_eq(pos->first, key));
    return pos->second;
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::mapped_type const&
ordered_map<Key, T, Compare>::at(key_type const& key) const
{
    const_iterator const pos = lower_bound(key);
    estd_assert((pos != end()) && key_eq(pos->first, key));
    return pos->second;
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::iterator
ordered_map<Key, T, Compare>::find(key_type const& key)
{
    iterator const pos = lower_bound(key);
    if ((pos != end()) && key_eq(pos->first, key))
    {
        return pos;
    }

    return end();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_iterator
ordered_map<Key, T, Compare>::find(key_type const& key) const
{
    const_iterator const pos = lower_bound(key);
    if ((pos != end()) && key_eq(pos->first, key))
    {
        return pos;
    }

    return end();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::iterator
ordered_map<Key, T, Compare>::lower_bound(key_type const& key)
{
    size_type const pos = lower_bound_search(key);
    if (pos >= max_size())
    {
        return end();
    }
    return begin() + pos;
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_iterator
ordered_map<Key, T, Compare>::lower_bound(key_type const& key) const
{
    size_type const pos = lower_bound_search(key);
    if (pos >= max_size())
    {
        return cend();
    }
    return cbegin() + pos;
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::iterator
ordered_map<Key, T, Compare>::upper_bound(key_type const& key)
{
    size_type const pos = upper_bound_search(key);
    if (pos >= max_size())
    {
        return end();
    }

    return begin() + pos;
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_iterator
ordered_map<Key, T, Compare>::upper_bound(key_type const& key) const
{
    size_type const pos = upper_bound_search(key);
    if (pos >= max_size())
    {
        return cend();
    }

    return cbegin() + pos;
}

template<class Key, class T, class Compare>
inline std::pair<
    typename ordered_map<Key, T, Compare>::iterator,
    typename ordered_map<Key, T, Compare>::iterator>
ordered_map<Key, T, Compare>::equal_range(key_type const& key)
{
    return std::make_pair(lower_bound(key), upper_bound(key));
}

template<class Key, class T, class Compare>
inline std::pair<
    typename ordered_map<Key, T, Compare>::const_iterator,
    typename ordered_map<Key, T, Compare>::const_iterator>
ordered_map<Key, T, Compare>::equal_range(key_type const& key) const
{
    return std::make_pair(lower_bound(key), upper_bound(key));
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::key_compare
ordered_map<Key, T, Compare>::key_comp() const
{
    return _comp;
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::value_compare
ordered_map<Key, T, Compare>::value_comp() const
{
    return value_compare(_comp);
}

// explicit
template<class Key, class T, class Compare>
inline ordered_map<Key, T, Compare>::value_compare::value_compare(Compare const c) : _valueComp(c)
{}

template<class Key, class T, class Compare>
inline bool ordered_map<Key, T, Compare>::value_compare::operator()(
    first_argument_type const& x, second_argument_type const& y) const
{
    return _valueComp(x.first, y.first);
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::iterator ordered_map<Key, T, Compare>::begin()
{
    return _values.begin();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_iterator
ordered_map<Key, T, Compare>::begin() const
{
    return _values.begin();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_iterator
ordered_map<Key, T, Compare>::cbegin() const
{
    return begin();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::iterator ordered_map<Key, T, Compare>::end()
{
    return _values.end();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_iterator
ordered_map<Key, T, Compare>::end() const
{
    return _values.end();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_iterator
ordered_map<Key, T, Compare>::cend() const
{
    return end();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::reverse_iterator
ordered_map<Key, T, Compare>::rbegin()
{
    return _values.rbegin();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_reverse_iterator
ordered_map<Key, T, Compare>::rbegin() const
{
    return _values.rbegin();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_reverse_iterator
ordered_map<Key, T, Compare>::crbegin() const
{
    return _values.crbegin();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::reverse_iterator ordered_map<Key, T, Compare>::rend()
{
    return _values.rend();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_reverse_iterator
ordered_map<Key, T, Compare>::rend() const
{
    return _values.rend();
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::const_reverse_iterator
ordered_map<Key, T, Compare>::crend() const
{
    return _values.crend();
}

template<class Key, class T, class Compare>
typename ordered_map<Key, T, Compare>::size_type
ordered_map<Key, T, Compare>::lower_bound_search(key_type const& key) const
{
    size_type pos;
    size_type first       = 0U;
    difference_type total = static_cast<difference_type>(_values.size());
    difference_type step;

    while (total > 0)
    {
        pos  = first;
        step = total / 2;
        pos += static_cast<size_type>(step);
        if (_comp(_values[pos].first, key) == true)
        {
            pos++;
            first = pos;
            total -= step + 1;
        }
        else
        {
            total = step;
        }
    }
    return first;
}

template<class Key, class T, class Compare>
typename ordered_map<Key, T, Compare>::size_type
ordered_map<Key, T, Compare>::upper_bound_search(key_type const& key) const
{
    size_type pos;
    size_type first       = 0U;
    difference_type total = static_cast<difference_type>(_values.size());
    difference_type step;

    while (total > 0)
    {
        pos  = first;
        step = total / 2;
        pos += static_cast<size_type>(step);
        if (_comp(key, _values[pos].first) == false)
        {
            pos++;
            first = pos;
            total -= step + 1;
        }
        else
        {
            total = step;
        }
    }
    return first;
}

template<class Key, class T, class Compare>
inline typename ordered_map<Key, T, Compare>::value_type&
ordered_map<Key, T, Compare>::emplace_uninitialized(ordered_map::iterator const pos)
{
    estd_assert(!full());
    if (pos == cend())
    {
        return *(_values.emplace_back().release_uninitialized_as());
    }

    // move all elements after pos to the back by one
    auto dstItr = end() - 1;
    auto srcItr = dstItr - 1;
    _values.emplace_back().construct(::std::move(*dstItr)); // construct new last element
    dstItr->~value_type();                                  // destroy previous last element
    while (dstItr != pos)
    {
        // move src to dest and destroy src
        (void)constructor<ordered_map::value_type>(
            type_utils<ordered_map::value_type>::cast_to_raw(dstItr))
            .construct(::std::move(*srcItr));
        srcItr->~value_type();
        --dstItr;
        --srcItr;
    }
    return *pos;
}

template<class Key, class T, class Compare>
inline bool ordered_map<Key, T, Compare>::key_eq(key_type const& x, key_type const& y) const
{
    return (_comp(x, y) == false) && (_comp(y, x) == false);
}

namespace declare
{
// explicit
template<class Key, class T, std::size_t N, class Compare>
ordered_map<Key, T, N, Compare>::ordered_map(key_compare const& comp)
: ::estd::ordered_map<Key, T, Compare>(_realValues, comp), _realValues()
{}

template<class Key, class T, std::size_t N, class Compare>
ordered_map<Key, T, N, Compare>::ordered_map(base const& other)
: ::estd::ordered_map<Key, T>(_realValues, other.key_comp()), _realValues()
{
    base::operator=(other);
}

template<class Key, class T, std::size_t N, class Compare>
ordered_map<Key, T, N, Compare>::ordered_map(this_type const& other)
: ::estd::ordered_map<Key, T>(_realValues, other.key_comp()), _realValues()
{
    base::operator=(other);
}

template<class Key, class T, std::size_t N, class Compare>
ordered_map<Key, T, N, Compare>::~ordered_map()
{
    base::clear();
}

template<class Key, class T, std::size_t N, class Compare>
ordered_map<Key, T, N, Compare>& ordered_map<Key, T, N, Compare>::operator=(base const& other)
{
    base::operator=(other);
    return *this;
}

template<class Key, class T, std::size_t N, class Compare>
ordered_map<Key, T, N, Compare>& ordered_map<Key, T, N, Compare>::operator=(this_type const& other)
{
    base::operator=(other);
    return *this;
}

} // namespace declare
} // namespace estd
