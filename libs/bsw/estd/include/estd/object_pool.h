// Copyright 2024 Accenture.

#pragma once

#include "estd/assert.h"
#include "estd/constructor.h"
#include "estd/type_utils.h"
#include "estd/uncopyable.h"

#include <platform/estdint.h>

#include <cstring>
#include <iterator>

namespace estd
{

template<class T>
class object_pool_iterator;
template<class T>
class object_pool_const_iterator;

/**
 * The object_pool class is designed to store a pool of objects.
 * [TPARAMS_BEGIN:object_pool]
 * \tparam  T   Type of elements of this object_pool.
 * [TPARAMS_END:object_pool]
 *
 * \see estd::declare::object_pool
 */
template<class T>
class object_pool
{
    UNCOPYABLE(object_pool);

protected:
    ~object_pool() = default;

public:
    // [PUBLIC_TYPES_BEGIN:object_pool]
    /** The template parameter T */
    using value_type      = T;
    /** A reference to the template parameter T */
    using reference       = T&;
    /** A pointer to the template parameter T */
    using pointer         = T*;
    /** A const reference to the template parameter T */
    using const_reference = T const&;
    /** A const pointer to the template parameter T */
    using const_pointer   = T const*;
    /** An unsigned integral type for sizes */
    using size_type       = std::size_t;
    /** A signed integral type */
    using difference_type = std::ptrdiff_t;
    /** An iterator over all acquired elements of this object_pool */
    using iterator        = object_pool_iterator<value_type>;
    /** A const_iterator over all acquired elements of this object_pool */
    using const_iterator  = object_pool_const_iterator<value_type>;
    // [PUBLIC_TYPES_END:object_pool]

    // [PUBLIC_API_BEGIN:object_pool]
    /**
     * Returns whether this object pool is depleted and can not be used to acquire additional
     * objects.
     */
    bool empty() const;

    /**
     * Returns true if none of this object_pool's objects have been acquired.
     */
    bool full() const;

    /**
     * Returns the number of objects that can be acquired from this object_pool.
     */
    size_type size() const;

    /**
     * Returns the maximum number of objects this object_pool can hold.
     */
    size_type max_size() const;

    /**
     * Returns whether a given object belongs to this object_pool's internal memory.
     */
    bool contains(const_reference object) const;

    /**
     * Acquires an object from this object_pool (default constructing it)
     * and returns a reference to it decreasing this object_pool's size by one.
     * \assert{!empty()}
     */
    reference acquire();

    /**
     * Returns a constructor object to the memory allocated for an object of this object_pool
     * decreasing this object_pool's size by one.
     * \assert{!empty()}
     */
    constructor<T> allocate();

    /**
     * Releases an acquired object to this pool (destroying it) increasing this object_pool's
     * size by one.
     * It is checked if the object being released belongs to this object_pool.
     */
    void release(const_reference object);

    /**
     * Releases all acquired objects in this pool. Calls the destructors
     * on all allocated objects. Any references to objects acquired from
     * this pool will be invalidated after this call.
     */
    void clear();

    /** Returns an iterator to the beginning of the released/allocated elements. */
    iterator begin();

    /** Returns a const iterator to the beginning of the released/allocated elements. */
    const_iterator begin() const;

    /** Returns a const iterator to the beginning of the released/allocated elements. */
    const_iterator cbegin() const;

    /** Returns an iterator to the end. */
    iterator end();

    /** Returns a const iterator to the end. */
    const_iterator end() const;

    /** Returns a const iterator to the end. */
    const_iterator cend() const;
    // [PUBLIC_API_END:object_pool]
protected:
    /**
     * Constructor to initialize this object_pool with the actual data to use.
     * Used by the declare class.
     *
     * \param data The actual data for this object pool
     * \param used The data for the used array
     * \param maxSize The size of the arrays
     *
     * \see estd::declare::object_pool
     */
    object_pool(uint8_t data[], uint8_t used[], size_type maxSize);

private:
    template<class>
    friend class object_pool_iterator;
    template<class>
    friend class object_pool_const_iterator;

    bool isUsed(size_type pos) const;

    void mark_used(size_type pos);

    void mark_unused(size_type pos);

    value_type* next(value_type const* current) const;

    uint8_t* _data;
    uint8_t* _used;
    size_type _size;
    size_type const _max_size;
};

/**
 * Iterator for object_pool.
 */
template<class T>
class object_pool_iterator
{
public:
    // [PUBLIC_TYPES_BEGIN:object_pool_iterator]
    using iterator_category = ::std::forward_iterator_tag;
    using value_type        = T;
    using reference         = T&;
    using pointer           = T*;
    using difference_type   = ::std::ptrdiff_t;
    // [PUBLIC_TYPES_END:object_pool_iterator]

    // [PUBLIC_API_BEGIN:object_pool_iterator]
    /** Creates an uninitialized iterator */
    object_pool_iterator();

    /** Dereferences the underlying object at the current iterator position */
    reference operator*() const;

    /** Dereferences the underlying object at the current iterator position */
    pointer operator->() const;

    /** Moves the iterator forward by one position */
    object_pool_iterator& operator++();

    /** Returns an iterator moved forward by one position */
    object_pool_iterator operator++(int);
    // [PUBLIC_API_END:object_pool_iterator]
private:
    template<class>
    friend class object_pool;
    template<class>
    friend class object_pool_const_iterator;

    object_pool_iterator(value_type* const current, object_pool<T> const* const pool)
    : _pool(pool), _current(current)
    {}

    object_pool<T> const* const _pool;
    value_type* _current;

public:
    /**
     * Compares two object_pool_iterators and returns true if they
     * point to the same element.
     *
     * \param lhs The left-hand side of the equality.
     * \param rhs The right-hand side of the equality.
     * \return
     * - true if the iterators pointer to the same element
     * - false otherwise.
     */
    friend inline bool operator==(object_pool_iterator const& lhs, object_pool_iterator const& rhs)
    {
        return (lhs._current == rhs._current);
    }

    /**
     * Compares two object_pool_iterators and returns true if they
     * point to different elements.
     *
     * \param lhs The left-hand side of the equality.
     * \param rhs The right-hand side of the equality.
     * \return
     * - true if the iterators pointer to different elements
     * - false otherwise.
     */
    friend inline bool operator!=(object_pool_iterator const& lhs, object_pool_iterator const& rhs)
    {
        return !(lhs == rhs);
    }
};

template<class T>
object_pool_iterator<T>::object_pool_iterator() : object_pool_iterator(nullptr, nullptr)
{}

template<class T>
typename object_pool_iterator<T>::reference object_pool_iterator<T>::operator*() const
{
    return *static_cast<T*>(_current);
}

template<class T>
typename object_pool_iterator<T>::pointer object_pool_iterator<T>::operator->() const
{
    return static_cast<T*>(_current);
}

template<class T>
object_pool_iterator<T>& object_pool_iterator<T>::operator++()
{
    _current = _pool->next(_current);
    return *this;
}

template<class T>
object_pool_iterator<T> object_pool_iterator<T>::operator++(int)
{
    object_pool_iterator const tmp(*this);
    (void)this->operator++();
    return tmp;
}

/**
 * Const iterator for object_pool.
 */
template<class T>
class object_pool_const_iterator
{
public:
    // [PUBLIC_TYPES_BEGIN:object_pool_const_iterator]
    using iterator_category = ::std::forward_iterator_tag;
    using value_type        = T const;
    using reference         = T const&;
    using pointer           = T const*;
    using difference_type   = ::std::ptrdiff_t;
    // [PUBLIC_TYPES_END:object_pool_const_iterator]

    // [PUBLIC_API_BEGIN:object_pool_const_iterator]
    /** Create an uninitialized iterator */
    object_pool_const_iterator();

    /**
     * A const object_pool iterator is constructable from a non-const version, implicit conversion
     * intended.
     */
    object_pool_const_iterator(::estd::object_pool_iterator<T> const& i);

    /** Dereferences the underlying object at the current iterator position. */
    reference operator*() const;

    /** Dereferences the underlying object at the current iterator position. */
    pointer operator->() const;

    /** Moves the iterator forward by one position. */
    object_pool_const_iterator& operator++();

    /** Returns an iterator moved forward by one position. */
    object_pool_const_iterator operator++(int);
    // [PUBLIC_API_END:object_pool_const_iterator]

private:
    template<class>
    friend class object_pool;

    friend inline bool
    operator==(object_pool_const_iterator const& x, object_pool_const_iterator const& y)
    {
        return (x._current == y._current);
    }

    friend inline bool
    operator!=(object_pool_const_iterator const& x, object_pool_const_iterator const& y)
    {
        return !(x == y);
    }

    object_pool_const_iterator(value_type* const current, object_pool<T> const* const pool)
    : _pool(pool), _current(current)
    {}

    object_pool<T> const* const _pool;
    value_type* _current;
};

template<class T>
object_pool_const_iterator<T>::object_pool_const_iterator()
: object_pool_const_iterator(nullptr, nullptr)
{}

template<class T>
object_pool_const_iterator<T>::object_pool_const_iterator(::estd::object_pool_iterator<T> const& i)
: object_pool_const_iterator(i._current, i._pool)
{}

template<class T>
typename object_pool_const_iterator<T>::reference object_pool_const_iterator<T>::operator*() const
{
    return *static_cast<T const*>(_current);
}

template<class T>
typename object_pool_const_iterator<T>::pointer object_pool_const_iterator<T>::operator->() const
{
    return static_cast<T const*>(_current);
}

template<class T>
object_pool_const_iterator<T>& object_pool_const_iterator<T>::operator++()
{
    _current = _pool->next(_current);
    return *this;
}

template<class T>
object_pool_const_iterator<T> object_pool_const_iterator<T>::operator++(int)
{
    object_pool_const_iterator const tmp(*this);
    (void)this->operator++();
    return tmp;
}

namespace declare
{
/**
 * The actual implementation of the object_pool class.
 * Use this to declare your object_pool, but use the base
 * class in your API.
 * [TPARAMS_BEGIN:declare_object_pool]
 * \tparam  T   Type of elements of this object_pool.
 * \tparam  N   Number of elements this object_pool manages.
 * [TPARAMS_END:declare_object_pool]
 */
template<class T, std::size_t N>
class object_pool : public ::estd::object_pool<T>
{
public:
    /** A shortcut to the base class */
    using base            = ::estd::object_pool<T>;
    /** The template parameter T */
    using value_type      = typename base::value_type;
    /** A reference to the template parameter T */
    using reference       = typename base::reference;
    /** A pointer to the template parameter T */
    using pointer         = typename base::pointer;
    /** A const reference to the template parameter T */
    using const_reference = typename base::const_reference;
    /** A const pointer to the template parameter T */
    using const_pointer   = typename base::const_pointer;
    /** An unsigned integral type for sizes */
    using size_type       = typename base::size_type;
    /** A signed integral type */
    using difference_type = typename base::difference_type;

    // [PUBLIC_API_BEGIN:declare_object_pool]
    /**
     * Constructs a full object_pool.
     */
    object_pool();

    // [PUBLIC_API_END:declare_object_pool]
private:
    ::uint8_t _realData[N * sizeof(T)];
    ::uint8_t _realUsed[(N + 7U) >> 3U];
};

} // namespace declare

/*
 *
 * Implementation
 *
 */

template<class T>
object_pool<T>::object_pool(uint8_t* const data, uint8_t* const used, size_type const maxSize)
: _data(data), _used(used), _size(maxSize), _max_size(maxSize)
{}

template<class T>
inline typename object_pool<T>::size_type object_pool<T>::size() const
{
    return _size;
}

template<class T>
inline typename object_pool<T>::size_type object_pool<T>::max_size() const
{
    return _max_size;
}

template<class T>
inline bool object_pool<T>::empty() const
{
    return (0U == _size);
}

template<class T>
inline bool object_pool<T>::full() const
{
    return (_max_size == _size);
}

template<class T>
typename object_pool<T>::reference object_pool<T>::acquire()
{
    for (size_type i = 0U; i < _max_size; ++i)
    {
        if (!isUsed(i))
        {
            --_size;
            mark_used(i);
            return constructor<T>(_data, i).construct();
        }
    }
    estd_assert(false);
    // will never be called!
    return constructor<T>(_data, 0).construct();
}

template<class T>
constructor<T> object_pool<T>::allocate()
{
    for (size_type i = 0U; i < _max_size; ++i)
    {
        if (!isUsed(i))
        {
            --_size;
            mark_used(i);
            return constructor<T>(_data, i);
        }
    }
    estd_assert(!empty());
    // will never be called
    return constructor<T>(nullptr);
}

template<class T>
inline bool object_pool<T>::contains(const_reference object) const
{
    return (
        (&object >= type_utils<T>::cast_const_to_type(_data))
        && (&object <= type_utils<T>::cast_const_to_type(_data, max_size() - 1U)));
}

template<class T>
void object_pool<T>::release(const_reference object)
{
    if (contains(object))
    {
        size_type const pos = static_cast<size_type>(
            std::distance(type_utils<T>::cast_const_to_type(_data), &object));
        if (isUsed(pos))
        {
            (void)object.T::~T();
            ++_size;
            mark_unused(pos);
        }
    }
}

template<class T>
void object_pool<T>::clear()
{
    for (size_type i = 0U; i < max_size(); ++i)
    {
        if (isUsed(i))
        {
            type_utils<T>::destroy(_data, i);
            mark_unused(i);
        }
    }
    _size = _max_size;
}

template<class T>
inline typename object_pool<T>::iterator object_pool<T>::begin()
{
    for (size_type i = 0U; i < max_size(); ++i)
    {
        if (isUsed(i))
        {
            return iterator(type_utils<T>::cast_to_type(_data, i), this);
        }
    }
    return iterator(nullptr, this);
}

template<class T>
inline typename object_pool<T>::const_iterator object_pool<T>::begin() const
{
    return const_iterator(const_cast<object_pool<T>*>(this)->begin());
}

template<class T>
inline typename object_pool<T>::const_iterator object_pool<T>::cbegin() const
{
    return begin();
}

template<class T>
inline typename object_pool<T>::iterator object_pool<T>::end()
{
    return iterator(nullptr, this);
}

template<class T>
inline typename object_pool<T>::const_iterator object_pool<T>::end() const
{
    return const_iterator(nullptr, this);
}

template<class T>
inline typename object_pool<T>::const_iterator object_pool<T>::cend() const
{
    return end();
}

template<class T>
inline bool object_pool<T>::isUsed(size_type const pos) const
{
    size_type const i     = static_cast<size_type>(pos / 8U);
    size_type const shift = static_cast<size_type>(pos % 8U);
    uint8_t const m       = static_cast<uint8_t>(1U << shift);
    return ((_used[i] & m) != 0U);
}

template<class T>
inline void object_pool<T>::mark_used(size_type const pos)
{
    size_type const i     = static_cast<size_type>(pos / 8U);
    size_type const shift = static_cast<size_type>(pos % 8U);
    uint8_t const m       = static_cast<uint8_t>(1U << shift);
    _used[i] |= m;
}

template<class T>
inline void object_pool<T>::mark_unused(size_type const pos)
{
    size_type const i     = static_cast<size_type>(pos / 8U);
    size_type const shift = static_cast<size_type>(pos % 8U);
    uint8_t const m       = ~static_cast<uint8_t>(1U << shift);
    _used[i] &= m;
}

template<class T>
inline typename object_pool<T>::value_type*
object_pool<T>::next(value_type const* const current) const
{
    if (current == nullptr)
    {
        return nullptr;
    }
    if (!contains(*current))
    {
        return nullptr;
    }
    size_type const offset = static_cast<size_type>(current - type_utils<T>::cast_to_type(_data));
    for (size_type i = offset + 1; i < max_size(); ++i)
    {
        if (isUsed(i))
        {
            return type_utils<T>::cast_to_type(_data, i);
        }
    }
    return nullptr;
}

namespace declare
{
template<class T, std::size_t N>
object_pool<T, N>::object_pool() : base(&_realData[0], &_realUsed[0], N)
{
    (void)memset(&_realUsed, 0x00, (N + 7U) >> 3);
}
} // namespace declare
} // namespace estd

