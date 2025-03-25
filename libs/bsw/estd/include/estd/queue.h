// Copyright 2024 Accenture.

/**
 * Contains estd::queue and estd::declare::queue.
 * \file
 * \ingroup estl_containers
 */
#pragma once

#include "estd/constructor.h"
#include "estd/deque.h"

#include <platform/estdint.h>

namespace estd
{
/**
 * A fixed-size container that holds a specific number of elements ordered
 * in a FIFO (first-in-first-out) sequence.
 *
 * \tparam T type of elements stored in the queue.
 * \tparam Container The underlying container object for storing the elements.
 *                   The default is the deque.
 *
 * \section queue_memory_usage Memory Usage
 *
 * The memory usage in bytes for this queue is determined by the underlying
 * container. By default the queue uses a deque as the storage. This is
 * done for performance. But it does come at a slight cost in terms of
 * memory usage.
 *
 * When using the deque as the underlying container:
 *
 * <b>32 bit OS</b>: (N + 1) * sizeof(T) + 20\n
 * <b>64 bit OS</b>: (N + 1) * sizeof(T) + 40
 *
 * \note It is possible there will be extra bytes used
 *       for proper memory alignment depending on your compiler settings.
 *
 * \section queue_example Usage example
 * \code{.cpp}
 * void processQueue(estd::queue<int>& q)
 * {
 *     while(!q.empty())
 *     {
 *         int item = q.front();
 *         // do something with item
 *         q.pop();
 *     }
 * }\endcode
 *
 * This container supports adding objects of type T that do not have
 * a copy constructor or default constructor. Use the emplace method to
 * push a complex object onto the queue.
 *
 * \section queue_emplace_example Emplace example
 * \code{.cpp}
 * void addEmplaceObject(estd::queue<ComplexObject>& q)
 * {
 *     // construct an instance of ComplexObject in place in the
 *     // queue. Calls the constructor: ComplexObject(1, 2, 3);
 *     q.emplace().construct(1, 2, 3);
 * }\endcode
 *
 * \see estd::declare::queue
 * \see estd::constructor
 */
template<class T, class Container = ::estd::deque<T>>
class queue
{
public:
    /** A typedef for this container */
    using this_type       = queue<T, Container>;
    /** The template parameter T */
    using value_type      = T;
    /** A reference to the template parameter T */
    using reference       = T&;
    /** A const reference to the template parameter T */
    using const_reference = T const&;
    /** The underlying container implementation */
    using container_type  = Container;
    /** An unsigned integral type for the size */
    using size_type       = std::size_t;

    /**
     * A simple destructor. This destructor will not call the contained
     * object destructors. The derived class will call the destructors
     * on any objects in the queue.
     *
     * \see estd::declare::queue
     */
    ~queue() = default;

    /**
     * An assignment operator.
     *
     * \param other The other queue to copy.
     * \return A reference to this.
     */
    this_type& operator=(this_type const& other)
    {
        _data = other._data;
        return *this;
    }

    /**
     * Returns whether this queue is empty or not.
     *
     * \return
     * - true if size() == 0
     * - false otherwise.
     */
    bool empty() const { return _data.empty(); }

    /**
     * Returns whether this queue is empty or not.
     *
     * \return
     * - true if size() == max_size()
     * - false otherwise.
     */
    bool full() const { return _data.full(); }

    /** Returns the number of elements in this queue */
    size_type size() const { return _data.size(); }

    /** Returns the maximum number of elements this queue can hold */
    size_type max_size() const { return _data.max_size(); }

    /**
     * Inserts a new element at the end of this queue increasing this queue's size by one.
     * The new element will be copy constructed from v.
     *
     * \param value The value to insert.
     * \assert{!full()}
     */
    void push(const_reference value) { _data.push_back(value); }

    /**
     * Returns a reference to the new last element of the queue (default constructed),
     * effectively increasing this queue's size by one.
     *
     * \return A reference to a newly constructed object.
     * \assert{!full()}
     */
    reference push() { return _data.push_back(); }

    /**
     * Returns a constructor object to the memory allocated for the last element of this
     * queue increasing this queue's size by one.
     *
     * \return A constructor object to construct a new element of T.
     * \assert{!full()}
     * \complexity  Constant.
     */
    constructor<T> emplace() { return _data.emplace_back(); }

    /**
     * Returns a reference to the current front of the queue.
     *
     * \return A reference to the front of the queue.
     * \assert{!empty()}
     */
    reference front() { return _data.front(); }

    /**
     * Returns a const reference to the current front of the queue.
     *
     * \return A const reference to the front of the queue.
     * \assert{!empty()}
     */
    const_reference front() const { return static_cast<Container const&>(_data).front(); }

    /**
     * Returns a reference to the current back of the queue.
     *
     * \return A reference to the back of the queue.
     * \assert{!empty()}
     */
    reference back() { return _data.back(); }

    /**
     * Returns a const reference to the current back of the queue.
     *
     * \return A const reference to the back of the queue.
     * \assert{!empty()}
     */
    const_reference back() const { return static_cast<Container const&>(_data).back(); }

    /**
     * Removes the first element of the queue, effectively decreasing its size by one.
     */
    void pop() { _data.pop_front(); }

    /**
     * Compares two queues and returns true if they are identical.
     *
     * Requires T to have operator== defined.
     *
     * \param lhs The left-hand side of the equality.
     * \param rhs The right-hand side of the equality.
     * \return
     * - true if the queues are identical
     * - false otherwise.
     */
    friend inline bool operator==(this_type const& lhs, this_type const& rhs)
    {
        return lhs._data == rhs._data;
    }

    /**
     * Compares two queues and returns true if they are not identical.
     *
     * Requires T to have operator== defined.
     *
     * \param lhs The left-hand side of the equality.
     * \param rhs The right-hand side of the equality.
     * \return
     * - true if the queues are not identical
     * - false otherwise.
     */
    friend inline bool operator!=(this_type const& lhs, this_type const& rhs)
    {
        return lhs._data != rhs._data;
    }

    /**
     * Compares two queues and returns true if all elements
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
    friend inline bool operator<(this_type const& lhs, this_type const& rhs)
    {
        return lhs._data < rhs._data;
    }

    /**
     * Compares two queues and returns true if all elements
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
    friend inline bool operator>(this_type const& lhs, this_type const& rhs)
    {
        return lhs._data > rhs._data;
    }

    /**
     * Compares two queues and returns true if all elements
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
    friend inline bool operator>=(this_type const& lhs, this_type const& rhs)
    {
        return lhs._data >= rhs._data;
    }

    /**
     * Compares two queues and returns true if all elements
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
    friend inline bool operator<=(this_type const& lhs, this_type const& rhs)
    {
        return lhs._data <= rhs._data;
    }

protected:
    /**
     * Constructor that initializes the queue with the specified
     * implementation container.
     *
     * \param container The underlying Container implementation.
     */
    explicit queue(Container& container) : _data(container) {}

private:
    queue(this_type const& other) = delete;

    Container& _data;
};

namespace declare
{
/**
 * Declaring a statically-sized queue object of size N.
 *
 * \tparam T The type of the elements in this queue.
 * \tparam N The maximum number of elements in the queue.
 * \tparam Container The type of the underlying implementation container.
 */
template<class T, std::size_t N, class Container = ::estd::declare::deque<T, N>>
class queue : public ::estd::queue<T, typename Container::base>
{
public:
    /** A shortcut to the base class */
    using base      = ::estd::queue<T, typename Container::base>;
    /** A shortcut to this type */
    using this_type = ::estd::declare::queue<T, N, Container>;

    /** The template parameter T */
    using value_type      = typename base::value_type;
    /** A reference to the template parameter T */
    using reference       = typename base::reference;
    /** A const reference to the template parameter T */
    using const_reference = typename base::const_reference;
    /** The underlying container implementation */
    using container_type  = typename base::container_type;
    /** An unsigned integral type for the size */
    using size_type       = typename base::size_type;

    /**
     * Default queue constructor
     */
    queue() : base(_realData) {}

    /**
     * Copy constructor for another base class queue.
     *
     * \param other The other queue to copy from.
     */
    queue(base const& other) : base(_realData) { base::operator=(other); }

    /**
     * Copy constructor for a declared queue.
     *
     * \param other The other queue to copy from.
     */
    queue(this_type const& other) : base(_realData) { base::operator=(other); }

    /**
     * Calls the destructor on all allocated objects.
     */
    ~queue() = default;

    /**
     * Assignment operator. Copies values from other into this queue.
     *
     * \param other The other queue to copy from.
     * \return A reference to this.
     */
    this_type& operator=(base const& other)
    {
        base::operator=(other);
        return *this;
    }

    /**
     * Assignment operator. Copies values from other into this queue.
     *
     * \param other The other queue to copy from.
     * \return A reference to this.
     */
    this_type& operator=(this_type const& other)
    {
        base::operator=(other);
        return *this;
    }

private:
    Container _realData;
};
} // namespace declare
} // namespace estd

