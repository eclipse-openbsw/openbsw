// Copyright 2024 Accenture.

/**
 * Contains estd::stack and estd::declare::stack.
 * \file
 * \ingroup estl_containers
 */
#pragma once

#include "estd/constructor.h"
#include "estd/vector.h"

#include <platform/estdint.h>

namespace estd
{
/**
 * A fixed-size container that holds a specific number of elements stored
 * in a LIFO (last-in-first-out) sequence.
 *
 * \tparam  T   Type of elements in this stack.
 * \tparam  Container   Underlying container of this stack.
 *                      The default is the vector.
 *
 * \section stack_memory_usage Memory Usage
 *
 * The memory usage in bytes for this stack is determined by the underlying
 * container. By default the stack uses a vector as the storage. This is
 * done for performance.
 *
 * When using the vector as the underlying container:
 *
 * <b>32 bit OS</b>: N * sizeof(T) + 16\n
 * <b>64 bit OS</b>: N * sizeof(T) + 32
 *
 * \note It is possible there will be extra bytes used
 *       for proper memory alignment depending on your compiler settings.
 *
 * \section stack_example Usage example
 * \code{.cpp}
 * void fillStack(estd::stack<int>& s)
 * {
 *     for(int i = 0; i < 5; ++i)
 *     {
 *         s.push(i);
 *     }
 * }
 *
 * void processStack(estd::stack<int>& s)
 * {
 *     while(!s.empty())
 *     {
 *         int item = s.top();
 *         // do something with item
 *         s.pop();
 *     }
 * }\endcode
 *
 * This container supports adding objects of type T that do not have
 * a copy constructor or default constructor. Use the emplace method
 * to push a complex object onto the stack.
 *
 * \section stack_emplace_example Emplace example
 * \code{.cpp}
 * void pushEmplaceObject(estd::stack<ComplexObject>& s)
 * {
 *     // construct an instance of ComplexObject in place in the
 *     // stack. Calls the constructor: ComplexObject(1, 2, 3);
 *     s.emplace().construct(1, 2, 3);
 * }\endcode
 *
 * \see estd::declare::stack
 * \see estd::constructor
 */
template<class T, class Container = ::estd::vector<T>>
class stack
{
public:
    /** A typedef for this container */
    using this_type       = stack<T, Container>;
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
     * on any objects in the stack.
     *
     * \see estd::declare::stack
     */
    ~stack() = default;

    /**
     * An assignment operator.
     *
     * \param other The other stack to copy.
     *
     * \return A reference to this.
     */
    this_type& operator=(this_type const& other)
    {
        _data = other._data;
        return *this;
    }

    /**
     * Returns whether this stack is empty or not.
     *
     * \return
     * - true if size() == 0
     * - false otherwise.
     */
    bool empty() const { return _data.empty(); }

    /**
     * Returns whether this stack is empty or not.
     *
     * \return
     * - true if size() == max_size()
     * - false otherwise.
     */
    bool full() const { return _data.full(); }

    /** Returns the number of elements in this stack */
    size_type size() const { return _data.size(); }

    /** Returns the maximum number of elements this stack can hold */
    size_type max_size() const { return _data.max_size(); }

    /**
     * Inserts a new element at the top of the stack, above the current top element,
     * effectively increasing the stack's size by one. The new element will be copy constructed from
     * v.
     *
     * \param value The value is push onto the stack.
     * \assert{!full()}
     */
    void push(const_reference value) { _data.push_back(value); }

    /**
     * Returns a reference to the new top of the stack (default constructed), above the current top
     * element, effectively increasing the stack's size by one.
     *
     * \return A reference to a newly constructed object.
     * \assert{!full()}
     */
    reference push() { return _data.push_back(); }

    /**
     * Returns a constructor object to the memory allocated for the new top of this
     * stack increasing this stack's size by one.
     *
     * \return A constructor object to construct a new element of T.
     * \assert{!full()}
     * \complexity  Constant.
     */
    constructor<T> emplace() { return _data.emplace_back(); }

    /**
     * Returns a reference to the current top of the stack.
     *
     * \return A reference to the top of the stack.
     * \assert{!empty()}
     */
    reference top() { return _data.back(); }

    /**
     * Returns a const reference to the top of the stack.
     *
     * \return A const reference to the top of the stack.
     * \assert{!empty()}
     */
    const_reference top() const { return static_cast<Container const&>(_data).back(); }

    /**
     * Removes the top element of the stack (destroying it), effectively decreasing its size by one.
     */
    void pop() { _data.pop_back(); }

    /**
     * Compares two stacks and returns true if they are identical.
     *
     * Requires T to have operator== defined.
     *
     * \param lhs The left-hand side of the equality.
     * \param rhs The right-hand side of the equality.
     * \return
     * - true if the stacks are identical
     * - false otherwise.
     */
    friend inline bool operator==(this_type const& lhs, this_type const& rhs)
    {
        return lhs._data == rhs._data;
    }

    /**
     * Compares two stacks and returns true if they are not identical.
     *
     * Requires T to have operator== defined.
     *
     * \param lhs The left-hand side of the equality.
     * \param rhs The right-hand side of the equality.
     * \return
     * - true if the stacks are not identical
     * - false otherwise.
     */
    friend inline bool operator!=(this_type const& lhs, this_type const& rhs)
    {
        return lhs._data != rhs._data;
    }

    /**
     * Compares two stacks and returns true if all elements
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
     * Compares two stacks and returns true if all elements
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
     * Compares two stacks and returns true if all elements
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
     * Compares two stacks and returns true if all elements
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
     * Constructor that initializes the stack with the specified
     * implementation container.
     *
     * \param container The underlying Container implementation.
     */
    explicit stack(Container& container) : _data(container) {}

private:
    // private and not defined
    stack(this_type const& other) = delete;

    Container& _data;
};

namespace declare
{
/**
 * Type to declare a statically-sized stack object of size N.
 *
 * \tparam T The type of the elements in this stack.
 * \tparam N The maximum number of elements in the stack.
 * \tparam Container The type of the underlying implementation container.
 */
template<class T, std::size_t N, class Container = ::estd::declare::vector<T, N>>
class stack : public ::estd::stack<T, typename Container::base>
{
public:
    /** A shortcut to the base class */
    using base      = ::estd::stack<T, typename Container::base>;
    /** A shortcut to this type */
    using this_type = ::estd::declare::stack<T, N, Container>;

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
     * Constructs an empty stack.
     */
    stack() : base(_realData) {}

    /**
     * Copies the values of the other stack into this stack.
     */
    stack(base const& other) : base(_realData) { base::operator=(other); }

    /**
     * Copies the values of the other stack into this stack.
     */
    stack(this_type const& other) : base(_realData) { base::operator=(other); }

    /**
     * Calls the destructor on all contained objects.
     */
    ~stack() = default;

    /**
     * Copies the values of the other stack into this stack.
     */
    this_type& operator=(base const& other)
    {
        base::operator=(other);
        return *this;
    }

    /**
     * Copies the values of the other stack into this stack.
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
