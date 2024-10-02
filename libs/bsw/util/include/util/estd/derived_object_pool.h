// Copyright 2024 Accenture.

#ifndef GUARD_4CA82602_8DB4_4CFC_B496_CF723394E317
#define GUARD_4CA82602_8DB4_4CFC_B496_CF723394E317

#include "util/estd/block_pool.h"

#include <estd/assert.h>
#include <estd/constructor.h>
#include <estd/type_list.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace util
{
namespace estd
{
template<class T>
class derived_object_pool
{
public:
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

    /**
     * constructor.
     *
     * \param blockPool block pool to use for allocation
     */
    explicit derived_object_pool(block_pool& blockPool) : _blockPool(blockPool) {}

    derived_object_pool(derived_object_pool const&)            = delete;
    derived_object_pool& operator=(derived_object_pool const&) = delete;

    /**
     * Returns whether this object pool can be used to acquire additional objects.
     */
    bool empty() const { return _blockPool.empty(); }

    /**
     * Returns true if none of this object_pool's objects have been acquired.
     */
    bool full() const { return _blockPool.full(); }

    /**
     * Returns the number of objects that can be acquired from this object_pool.
     */
    size_type size() const { return _blockPool.size(); }

    /**
     * Returns the maximum number of objects this object_pool can hold.
     */
    size_type max_size() const { return _blockPool.max_size(); }

    /**
     * Returns whether a given object belongs to this object_pool's internal memory.
     */
    bool contains(const_reference object) const
    {
        return _blockPool.contains(reinterpret_cast<uint8_t const*>(&object));
    }

    /**
     * Returns a constructor object to the memory allocated for a (derived) object of this
     * derived_object_pool decreasing this object_pool's size by one.
     * \assert{!full()}
     */
    template<class D>
    ::estd::constructor<D> allocate();

    /**
     * Releases an acquired object to this pool (destroying it) increasing this object_pool's
     * size by one.
     * It is checked if the object being released belongs to this object_pool.
     */
    void release(reference object)
    {
        (void)object.~T();
        _blockPool.release(reinterpret_cast<uint8_t*>(&object));
    }

    /**
     * Get const reference to underlying block pool.
     */
    block_pool const& get_block_pool() const { return _blockPool; }

    /**
     * Get reference to underlying block pool.
     */
    block_pool& get_block_pool() { return _blockPool; }

private:
    block_pool& _blockPool;
};

namespace internal
{
template<class D, class T, bool IsDerived = std::is_base_of<T, D>::value>
struct _derived_type
{};

template<class D, class T>
struct _derived_type<D, T, true>
{
    using type = D;
};

template<typename T>
struct _append_type_list
{
    using type = ::estd::type_list<T>;
};

template<typename HEAD, typename TAIL>
struct _append_type_list<::estd::type_list<HEAD, TAIL>>
{
    using type = ::estd::type_list<HEAD, TAIL>;
};
} // namespace internal

template<class T>
template<class D>
::estd::constructor<D> derived_object_pool<T>::allocate()
{
    estd_assert(_blockPool.block_size() >= sizeof(typename internal::_derived_type<D, T>::type));
    return ::estd::constructor<D>(_blockPool.acquire());
}

namespace declare
{
/**
 * The actual implementation of the derived_object_pool class.
 * Use this to declare your derived_object_pool, but use the base
 * class in your API.
 * \tparam  T   Type of elements of this object_pool.
 * \tparam  N   Number of elements this object_pool manages.
 * \tparam  D   One or more derived types (use type_list for more than one)
 */
template<class T, std::size_t N, class D = T>
class derived_object_pool : public ::util::estd::derived_object_pool<T>
{
public:
    using derived_type_list = ::estd::type_list<T, typename internal::_append_type_list<D>::type>;

    derived_object_pool() : ::util::estd::derived_object_pool<T>(_blockPool) {}

private:
    block_pool<N, ::estd::max_size<derived_type_list>::value> _blockPool;
};

} // namespace declare
} // namespace estd
} // namespace util

#endif // GUARD_4CA82602_8DB4_4CFC_B496_CF723394E317
