// Copyright 2024 Accenture.

/**
 * Contains estd::priority_queue and estd::declare::priority_queue.
 * \file
 * \ingroup estl_containers
 */
#pragma once

#include "estd/constructor.h"
#include "estd/vector.h"

#include <platform/estdint.h>

#include <algorithm>
#include <functional>

namespace estd
{
/**
 * C++11 like implementation of a priority queue.
 * \tparam  T type of elements of the priority queue, aliased by typedef value_type.
 * \tparam  Comparator How to compare the elements in the priority queue
 * \tparam  Container Underlying storage implementation of the priority queue.
 *
 * \section priority_queue_memory_usage Memory Usage
 *
 * The memory usage in bytes for this priority_queue is determined
 * by the underlying container. By default the stack uses a vector
 * as the storage. This is done for performance.
 *
 * When using the vector as the underlying container:
 *
 * <b>32 bit OS</b>: N * sizeof(T) + 20\n
 * <b>64 bit OS</b>: N * sizeof(T) + 40
 *
 * \note It is possible there will be extra bytes used
 *       for proper memory alignment depending on your compiler settings.
 *
 */
template<class T, class Comparator = std::less<T>, class Container = ::estd::vector<T>>
class priority_queue
{
public:
    /** A typedef for this container. Used internally. */
    using this_type       = priority_queue<T, Comparator, Container>;
    /** The underlying container type. */
    using container_type  = Container;
    /** The template parameter T */
    using value_type      = typename container_type::value_type;
    /** A reference to the template parameter T */
    using reference       = typename container_type::reference;
    /** A const reference to the template parameter T */
    using const_reference = typename container_type::const_reference;

    /**
     * A simple destructor. This destructor does not call the contained
     * object destructors. The derived class priority_queue will call
     * the destructors.
     *
     * \see estd::declare::deque
     */
    ~priority_queue() = default;

    /**
     * An assignment operator. Copies the elements from other into
     * this priority_queue.
     *
     * T must provide a copy-constructor.
     *
     * \param other The other priority_queue to copy.
     * \return A reference to this.
     */
    this_type& operator=(this_type const& other)
    {
        _values = other._values;
        return *this;
    }

    /**
     * \return
     * - true if the priority queue is full
     * - false otherwise
     */
    bool full() const { return _values.full(); }

    /**
     * \return
     * - true if the priority queue is empty
     * - false otherwise.
     */
    bool empty() const { return _values.empty(); }

    /**
     * The current number of items in the priority queue.
     *
     * \return The current size of the priority queue.
     */
    std::size_t size() const { return _values.size(); }

    /**
     * The max_size of a priority queue equals N.
     *
     * \return The maximum size of this priority queue.
     */
    std::size_t max_size() const { return _values.max_size(); }

    /**
     * Returns the top of the priority queue.
     *
     * \return  A const reference to first element of priority queue
     * \assert{!empty()}
     */
    const_reference top() const { return _values.at(0U); }

    /**
     * Adds an element of type T to the priority queue
     *
     * \param value Element to add
     *
     * \assert{!full()}
     */
    void push(const_reference value)
    {
        (void)_values.emplace_back().construct(value);
        auto const b = _values.begin();
        auto const e = _values.end();
        std::push_heap(b, e, _compare);
    }

    /**
     * Removes the first element from priority queue
     *
     * \assert{!empty()}
     */
    void pop()
    {
        auto const b = _values.begin();
        auto const e = _values.end();
        std::pop_heap(b, e, _compare);
        _values.pop_back();
    }

protected:
    /**
     * Constructor that initializes the priority_queue with the specified
     * implementation container.
     *
     * \param container The underlying container implementation.
     */
    explicit priority_queue(Container& container) : _compare(), _values(container) {}

private:
    priority_queue(this_type const& other) = delete;

    Comparator _compare;
    Container& _values;
};

/*
 *
 * Class to declare a static priority queue of size N
 *
 */
namespace declare
{
/**
 * Declaring a statically-sized priority-queue object of size N.
 */
template<
    class T,
    std::size_t N,
    class Comparator = ::std::less<T>,
    class Container  = ::estd::declare::vector<T, N>>
class priority_queue : public ::estd::priority_queue<T, Comparator, typename Container::base>
{
public:
    /** A shortcut to the base class */
    using base      = ::estd::priority_queue<T, Comparator, typename Container::base>;
    /** A shortcut to this type */
    using this_type = ::estd::declare::priority_queue<T, N, Comparator, Container>;

    /** The underlying container type. */
    using container_type  = typename base::container_type;
    /** The template parameter T */
    using value_type      = typename base::value_type;
    /** A reference to the template parameter T */
    using reference       = typename base::reference;
    /** A const reference to the template parameter T */
    using const_reference = typename base::const_reference;

    /**
     * Constructs an empty priority_queue
     */
    priority_queue() : base(_realValues) {}

    /**
     * Copies the values of the other priority_queue into this priority_queue.
     */
    priority_queue(base const& other) : base(_realValues) { base::operator=(other); }

    /**
     * Copies the values of the other priority_queue into this priority_queue.
     */
    priority_queue(this_type const& other) : base(_realValues) { base::operator=(other); }

    /**
     * Calls the destructor on all contained objects.
     */
    ~priority_queue() = default;

    /**
     * Copies the values of the other priority_queue into this priority_queue.
     */
    this_type& operator=(base const& other)
    {
        base::operator=(other);
        return *this;
    }

    /**
     * Copies the values of the other priority_queue into this priority_queue.
     */
    this_type& operator=(this_type const& other)
    {
        base::operator=(other);
        return *this;
    }

private:
    Container _realValues;
};
} // namespace declare
} // namespace estd

