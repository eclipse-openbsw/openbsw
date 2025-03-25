// Copyright 2024 Accenture.

/**
 * Contains estd::deque and estd::declare::deque.
 * \file
 * \ingroup estl_containers
 */
#pragma once

#include "estd/algorithm.h"
#include "estd/assert.h"
#include "estd/constructor.h"
#include "estd/type_utils.h"

#include <platform/estdint.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <type_traits>

namespace estd
{
template<class ValueType>
class deque_iterator;
template<class ValueType>
class deque_const_iterator;

/**
 * A fixed-size container that holds a specific number of elements
 * which can be added or removed from both ends.
 *
 * \tparam  T   Type of values of this deque.
 *
 * \section deque_memory_usage Memory Usage
 *
 * The memory usage in bytes for this deque will be:
 *
 * <b>32 bit OS</b>: (N + 1) * sizeof(T) + 16\n
 * <b>64 bit OS</b>: (N + 1) * sizeof(T) + 32
 *
 * \note It is possible there will be extra bytes used
 *       for proper memory alignment depending on your compiler settings.
 *
 * \section deque_example Usage example
 * \code{.cpp}
 * void addItemsToDeque(estd::deque<int>& d)
 * {
 *     for(int i = 0; i < 10; ++i)
 *     {
 *         d.push_back(i);
 *     }
 * }
 *
 * void processDeque(estd::deque<int> const& d)
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
 * a copy constructor or default constructor. Use the emplace method
 * to create an instance of T directly into the deque.
 *
 * \section deque_emplace_example Emplace example
 * \code{.cpp}
 * void addEmplaceObject(estd::deque<ComplexObject>& d)
 * {
 *     // construct an instance of ComplexObject directly in the deque.
 *     // Calls the constructor: ComplexObject(1, 2, 3);
 *     d.emplace_back().construct(1, 2, 3);
 * }\endcode
 *
 * \see estd::declare::deque
 * \see estd::constructor
 */
template<class T>
class deque
{
public:
    /** Template parameter T */
    using value_type             = T;
    /** Reference to template parameter T */
    using reference              = T&;
    /** Const reference to template parameter T */
    using const_reference        = T const&;
    /** A pointer to type T */
    using pointer                = T*;
    /** A const pointer to type T */
    using const_pointer          = T const*;
    /** An unsigned integral type for sizes */
    using size_type              = std::size_t;
    /** A signed integral type */
    using difference_type        = std::ptrdiff_t;
    /** A random access iterator */
    using iterator               = deque_iterator<T>;
    /** A const random access iterator */
    using const_iterator         = deque_const_iterator<T>;
    /** A reverse iterator */
    using reverse_iterator       = std::reverse_iterator<iterator>;
    /** A const reverse iterator */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    deque(deque<T> const& other) = delete;

    /**
     * A simple destructor. This destructor does not call the contained
     * object destructors. The derived class deque will call the destructors.
     *
     * \see estd::declare::deque
     */
    ~deque() = default;

    /**
     * An assignment operator. Copies the elements from other into
     * this deque.
     *
     * T must provide a copy-constructor.
     *
     * \param other The other deque to copy.
     *
     * \return A reference to this.
     */
    deque& operator=(deque<T> const& other);

    /**
     * Replaces any element currently in this container and fills it with n
     * elements which all are initialized with a copy of value. Existing
     * elements are removed.
     *
     * \param n The number of elements to add
     * \param value The value to copy into this deque.
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
    typename std::enable_if<std::is_integral<InputIterator>::value == 0, void>::type
    assign(InputIterator first, InputIterator const last)
    {
        clear();
        while ((!full()) && (first != last))
        {
            T const& item = *first;
            (void)emplace_back().construct(item);
            ++first;
        }
    }

    /**
     * Returns the number of elements in this deque.
     *
     * \complexity  Constant.
     */
    size_type size() const;

    /**
     * Returns the maximum number of elements that this deque can hold.
     *
     * \complexity  Constant.
     */
    size_type max_size() const;

    /**
     * Returns whether this deque is empty or not.
     *
     * \complexity  Constant.
     */
    bool empty() const;

    /**
     * Returns whether this deque is full or not.
     *
     * \return
     * - true if size() == max_size()
     * - false otherwise
     * \complexity  Constant.
     */
    bool full() const;

    /**
     * Removes all elements (destroying them) from this deque leaving it
     * with a size of 0.
     *
     * \complexity  Constant.
     */
    void clear();

    /**
     * Copies a given element to this deque effectively increasing the size
     * by one. If the deque is already full, it will remain unchanged.
     *
     * \param value The new value to add to the deque.
     * \complexity  Constant.
     */
    void push_back(const_reference value);

    /**
     * Returns a reference to the new last element (default constructed) increasing the size
     * of this deque by one.
     *
     * \return A reference to a default constructed object
     * \assert{!full()}
     * \complexity  Constant.
     */
    reference push_back();

    /**
     * Returns a constructor object to the memory allocated at the end of this deque increasing
     * this deque's size by one.
     *
     * \return A constructor object that can be used to initialize an object.
     * \assert{!full()}
     * \complexity  Constant.
     */
    constructor<T> emplace_back();

    /**
     * Returns a reference to the last element of this deque.
     *
     * \return A reference to the last element
     * \assert{!empty()}
     * \complexity  Constant.
     */
    reference back();

    /**
     * Returns a read only reference to the last element of this deque.
     *
     * \return A const reference to the last element
     * \assert{!empty()}
     * \complexity  Constant.
     */
    const_reference back() const;

    /**
     * Copy constructs a given element to the front of this deque
     * increasing the size by one.
     *
     * \param value The value to add to the queue
     * \assert{!full()}
     * \complexity  Constant.
     */
    void push_front(const_reference value);

    /**
     * Returns a reference to the new first element (default constructed) increasing the
     * size of this deque by one.
     *
     * \return A reference to a default constructed object.
     * \assert{!full()}
     * \complexity  Constant.
     */
    reference push_front();

    /**
     * Returns a constructor object to the memory allocated for the new first element of this
     * deque increasing this deque's size by one.
     *
     * \return A constructor object that can be used to initialize an object.
     * \assert{!full()}
     * \complexity  Constant.
     */
    constructor<T> emplace_front();

    /**
     * Removes the last element (destroying it) of this deque decreasing its size by one.
     * \complexity  Constant.
     *
     * If this deque is empty, it will remain empty.
     */
    void pop_back();

    /**
     * Removes the first element (destroying it) of this deque decreasing its size by one.
     * \complexity  Constant.
     *
     * If this deque is empty, it will remain empty.
     */
    void pop_front();

    /**
     * Returns a reference to the first element of this deque.
     *
     * \return A reference to the first element in the deque.
     * \assert{!empty()}
     * \complexity  Constant.
     */
    reference front();

    /**
     * Returns a read only reference to the first element of this deque.
     *
     * \return a const reference to the first element in the deque.
     * \assert{!empty()}
     * \complexity  Constant.
     */
    const_reference front() const;

    /**
     * Returns a reference to the element at position 'n' in this deque.
     *
     * \param n An index into this deque.
     * \return A reference to the requested element.
     * \note
     * No bounds checking is done.
     */
    reference operator[](size_type n);

    /**
     * Returns a reference to the element at position 'n' in this deque.
     *
     * \param n An index into this deque.
     * \return A const reference to the requested element.
     * \note
     * No bounds checking is done.
     */
    const_reference operator[](size_type n) const;

    /**
     * Returns a reference to the element at position 'n' in this deque.
     *
     * \param n An index into this deque.
     * \return A reference to the requested element.
     * \assert{n < size()}
     */
    reference at(size_type n);

    /**
     * Returns a reference to the element at position 'n' in this deque.
     *
     * \param n An index into this deque.
     * \return A const reference to the requested element.
     * \assert{n < size()}
     */
    const_reference at(size_type n) const;

    /**
     * Resizes the container to n elements. If n is smaller than the current
     * size, the content is reduced to n elements. If n is greater than the
     * current size, the content is expanded and the newly inserted elements
     * are initialized with a default constructed value.
     *
     * \param n The new requested size of this deque.
     */
    void resize(size_type n);

    /**
     * Resizes the container to n elements. If n is smaller than the current
     * size, the content is reduced to n elements. If n is greater than the
     * current size, the content is expanded and the newly inserted elements
     * are initialized with value.
     *
     * \param n The new requested size of this deque.
     * \param value The value to copy if the deque is expanded.
     */
    void resize(size_type n, const_reference value);

    /**
     * Removes a single element from this deque to which position is pointing.
     * The destructor will be called on the element removed.
     *
     * \param position The item to remove.
     * \return  Iterator to the element following position.
     */
    iterator erase(const_iterator position);

    /**
     * Removes the range [first, last).
     *
     * \param first The first element in the range to erase.
     * \param last The last element in the range to erase.
     * \return  Iterator to the element following the last erased element.
     */
    iterator erase(const_iterator first, const_iterator last);

    /**
     * Returns a constructor object to memory allocated at the position
     * specified by the iterator.
     *
     * \param position The position where a new element should be allocated
     * \return A constructor object to initialize the new element.
     * \assert{!full()}
     *
     */
    constructor<T> emplace(const_iterator position);

    /**
     * This deque is extended by inserting a given value before the element at the specified
     * position.
     *
     * \param position The position where a new element should be created
     * \param value The default value for the new element
     * \return An iterator to the newly created element.
     */
    iterator insert(const_iterator position, const_reference value);

    /**
     * This deque is extended by inserting a given value n times before the element at the specified
     * position.
     *
     * \param position The position where the new elements should be added.
     * \param n The number of elements to add
     * \param value The value to copy into this deque
     */
    void insert(const_iterator position, size_type n, const_reference value);

    /**
     * Inserts elements in the range [first, last) into this deque
     * at 'position'.
     *
     * \param position The position where the new elements should be added
     * \param first The starting iterator
     * \param last The ending iterator
     * \return An iterator to the first element inserted
     */
    template<class InputIterator>
    typename std::enable_if<std::is_integral<InputIterator>::value == 0, iterator>::type
    insert(const_iterator position, InputIterator first, InputIterator const last)
    {
        iterator const start(this, position._current);

        while ((!full()) && (first != last))
        {
            T const& item = *first;

            position = const_iterator(deque::insert(position, item));
            ++first;
            ++position;
        }

        return start;
    }

    /** Returns an iterator to the beginning */
    iterator begin();

    /** Returns a const iterator to the beginning */
    const_iterator begin() const;

    /** Returns an iterator to the end */
    iterator end();

    /** Returns a const iterator to the end */
    const_iterator end() const;

    /**
     * Returns a reverse iterator pointing to the last element in the deque
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    reverse_iterator rbegin();

    /**
     * Returns a const reverse iterator pointing to the last element in the deque
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    const_reverse_iterator rbegin() const;

    /**
     * Returns a reverse iterator pointing to the first element in the deque
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    reverse_iterator rend();

    /**
     * Returns a const reverse iterator pointing to the first element in the deque
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    const_reverse_iterator rend() const;

    /** Return a const iterator to the beginning */
    const_iterator cbegin() const;

    /** Returns a const iterator to the end */
    const_iterator cend() const;

    /**
     * Returns a const reverse iterator pointing to the last element in the deque
     * (i.e., its reverse beginning). Reverse iterators iterate backwards.
     */
    const_reverse_iterator crbegin() const;

    /**
     * Returns a const reverse iterator pointing to the first element in the deque
     * (i.e., its reverse end). Reverse iterators iterate backwards.
     */
    const_reverse_iterator crend() const;

protected:
    /**
     * Constructor to initialize this deque with the actual data to work with.
     * Used by the declare class.
     *
     * \param data The actual data for this deque
     * \param N The size of the data array.
     */
    deque(uint8_t* data, size_type N);

private:
    static void callDestructors(const_iterator b, const_iterator e);
    bool canMoveHeadLeft(size_type current) const;
    void moveLeft(size_type destPos, size_type srcPos, size_type n);
    void moveRight(size_type destPos, size_type srcPos, size_type n);

    template<class>
    friend class deque_iterator;

    template<class>
    friend class deque_const_iterator;

    uint8_t* _data;
    size_type _num_elements;
    size_type _head;
    size_type _tail;
};

/**
 * Swaps two deques.
 *
 */

/**
 * Compares two deques and returns true if they are identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the deques are identical
 * - false otherwise.
 */
template<class T>
bool operator==(deque<T> const& lhs, deque<T> const& rhs);

/**
 * Compares two deques and returns true if they are not identical.
 *
 * Requires T to have operator== defined.
 *
 * \param lhs The left-hand side of the equality.
 * \param rhs The right-hand side of the equality.
 * \return
 * - true if the deques are not identical
 * - false otherwise.
 */
template<class T>
bool operator!=(deque<T> const& lhs, deque<T> const& rhs);

/**
 * Compares two deques and returns true if all elements
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
bool operator<(deque<T> const& lhs, deque<T> const& rhs);

/**
 * Compares two deques and returns true if all elements
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
bool operator>(deque<T> const& lhs, deque<T> const& rhs);

/**
 * Compares two deques and returns true if all elements
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
bool operator>=(deque<T> const& lhs, deque<T> const& rhs);

/**
 * Compares two deques and returns true if all elements
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
bool operator<=(deque<T> const& lhs, deque<T> const& rhs);

namespace declare
{
/**
 * deque with fixed size.
 * \tparam  T   Type of values of this deque.
 * \tparam  N   Maximum number of values in this deque.
 *
 * \note
 * This container will allocate N + 1 elements, because the N + 1st element
 * will be used for the end-iterator.
 */
template<class T, std::size_t N>
class deque : public ::estd::deque<T>
{
public:
    /** A shortcut to the base class */
    using base      = ::estd::deque<T>;
    /** A shortcut to this type */
    using this_type = ::estd::declare::deque<T, N>;

    /** Template parameter T */
    using value_type             = typename base::value_type;
    /** Reference to template parameter T */
    using reference              = typename base::reference;
    /** Const reference to template parameter T */
    using const_reference        = typename base::const_reference;
    /** A pointer to type T */
    using pointer                = typename base::pointer;
    /** A const pointer to type T */
    using const_pointer          = typename base::const_pointer;
    /** An unsigned integral type for sizes */
    using size_type              = typename base::size_type;
    /** A signed integral type */
    using difference_type        = typename base::difference_type;
    /** A random access iterator */
    using iterator               = typename base::iterator;
    /** A const random access iterator */
    using const_iterator         = typename base::const_iterator;
    /** A reverse iterator */
    using reverse_iterator       = typename base::reverse_iterator;
    /** A const reverse iterator */
    using const_reverse_iterator = typename base::const_reverse_iterator;

    /**
     * Constructs an empty deque.
     */
    deque();

    /**
     * Constructs a deque with n elements initialized to a default value_type.
     */
    explicit deque(size_type n);

    /**
     * Constructs a deque with n elements initialized to a given value value.
     */
    deque(size_type n, const_reference value);

    /**
     * Copies the values of the other deque into this deque.
     */
    deque(base const& other);
    /**
     * Copies the values of the other deque into this deque.
     */
    deque(this_type const& other);

    /**
     * Calls the destructor on all contained objects.
     */
    ~deque();

    /**
     * Copies the values of the other deque into this deque.
     */
    this_type& operator=(base const& other);

    /**
     * Copies the values of the other deque into this deque.
     */
    this_type& operator=(this_type const& other);

private:
    ::uint8_t _realData[sizeof(T) * (N + 1U)];
};

} // namespace declare

/**
 * A random access iterator for a std::deque.
 *
 */
template<class T>
class deque_iterator
{
public:
    using iterator_category = ::std::random_access_iterator_tag;
    using value_type        = T;
    using reference         = T&;
    using pointer           = T*;
    using difference_type   = ::std::ptrdiff_t;

    using container_type = ::estd::deque<T>;

    using size_type = typename container_type::size_type;

    deque_iterator() : _deque(nullptr), _current(0U) {}

    /** Dereferences the underlying object at the current iterator position */
    reference operator*() const;

    /** Dereferences the underlying object at the current iterator position */
    pointer operator->() const;

    /** Moves the iterator forward by one position */
    deque_iterator& operator++();

    /** Increments this iterator by one position, returning a copy of the previous position */
    deque_iterator operator++(int);

    /** Moves the iterator backwards by one position */
    deque_iterator& operator--();

    /** Decrements this iterator by one position, returning a copy of the previous position */
    deque_iterator operator--(int);

    /** Moves the iterator forward by 'n' places */
    deque_iterator& operator+=(difference_type n);

    /** Moves the iterator backwards by 'n' places */
    deque_iterator& operator-=(difference_type n);

    /** Returns a new iterator that is 'n' places ahead */
    deque_iterator operator+(difference_type n) const;

    /** Returns a new iterator that is 'n' places behind */
    deque_iterator operator-(difference_type n) const;

    /** Returns the distance between this iterator and 'other' */
    difference_type operator-(deque_iterator const& other) const;

    /** Returns a reference to an item at index 'n' */
    reference operator[](difference_type n) const;

private:
    template<class>
    friend class deque;
    template<class>
    friend class deque_const_iterator;

    friend inline bool operator==(deque_iterator const& x, deque_iterator const& y)
    {
        return ((x._deque == y._deque) && (x._current == y._current));
    }

    friend inline bool operator!=(deque_iterator const& x, deque_iterator const& y)
    {
        return !(x == y);
    }

    friend inline bool operator<(deque_iterator const& x, deque_iterator const& y)
    {
        if (x._deque == y._deque)
        {
            return (x.absPosition() < y.absPosition());
        }
        return false;
    }

    friend inline bool operator<=(deque_iterator const& x, deque_iterator const& y)
    {
        if (x._deque == y._deque)
        {
            return (x.absPosition() <= y.absPosition());
        }
        return false;
    }

    friend inline bool operator>(deque_iterator const& x, deque_iterator const& y)
    {
        if (x._deque == y._deque)
        {
            return (y.absPosition() < x.absPosition());
        }
        return false;
    }

    friend inline bool operator>=(deque_iterator const& x, deque_iterator const& y)
    {
        if (x._deque == y._deque)
        {
            return (x.absPosition() >= y.absPosition());
        }
        return false;
    }

    pointer get() const;
    void move(difference_type n);
    difference_type absPosition() const;

    deque_iterator(container_type* const theDeque, size_type const current)
    : _deque(theDeque), _current(current)
    {}

    container_type* _deque;
    size_type _current;
};

/**
 * Returns a new iterator that is 'n' places ahead of 'other'.
 */
template<class T>
deque_iterator<T>
operator+(typename deque_iterator<T>::difference_type n, deque_iterator<T> const& other);

/**
 * A const random access iterator for a std::deque.
 */
template<class T>
class deque_const_iterator
{
public:
    using iterator_category = ::std::random_access_iterator_tag;
    using value_type        = T const;
    using reference         = T const&;
    using pointer           = T const*;
    using difference_type   = ::std::ptrdiff_t;

    using container_type = ::estd::deque<T>;

    using size_type = typename container_type::size_type;

    /** Create an uninitialized iterator */
    deque_const_iterator() = default;

    /** A const deque iterator is constructable from a non-const version */
    deque_const_iterator(::estd::deque_iterator<T> const& other)
    : _deque(other._deque), _current(other._current)
    {}

    /** Dereferences the underlying object at the current iterator position */
    reference operator*() const;

    /** Dereferences the underlying object at the current iterator position */
    pointer operator->() const;

    /** Moves the iterator forward by one position */
    deque_const_iterator& operator++();

    /** Increments this iterator by one position, returning a copy of the previous position */
    deque_const_iterator operator++(int);

    /** Moves the iterator backwards by one position */
    deque_const_iterator& operator--();

    /** Decrements this iterator by one position, returning a copy of the previous position */
    deque_const_iterator operator--(int);

    /** Moves the iterator forward by 'n' places */
    deque_const_iterator& operator+=(difference_type n);

    /** Moves the iterator backwards by 'n' places */
    deque_const_iterator& operator-=(difference_type n);

    /** Returns a new iterator that is 'n' places ahead */
    deque_const_iterator operator+(difference_type n) const;

    /** Returns a new iterator that is 'n' places behind */
    deque_const_iterator operator-(difference_type n) const;

    /** Returns the distance between this iterator and 'other' */
    difference_type operator-(deque_const_iterator const& other) const;

    /** Returns a reference to an item at index 'n' */
    reference operator[](difference_type n) const;

private:
    template<class>
    friend class deque;

    friend inline bool operator==(deque_const_iterator const& x, deque_const_iterator const& y)
    {
        return ((x._deque == y._deque) && (x._current == y._current));
    }

    friend inline bool operator!=(deque_const_iterator const& x, deque_const_iterator const& y)
    {
        return !(x == y);
    }

    friend inline bool operator<(deque_const_iterator const& x, deque_const_iterator const& y)
    {
        if (x._deque == y._deque)
        {
            return (x.absPosition() < y.absPosition());
        }
        return false;
    }

    friend inline bool operator<=(deque_const_iterator const& x, deque_const_iterator const& y)
    {
        if (x._deque == y._deque)
        {
            return (x.absPosition() <= y.absPosition());
        }
        return false;
    }

    friend inline bool operator>(deque_const_iterator const& x, deque_const_iterator const& y)
    {
        if (x._deque == y._deque)
        {
            return (y.absPosition() < x.absPosition());
        }
        return false;
    }

    friend inline bool operator>=(deque_const_iterator const& x, deque_const_iterator const& y)
    {
        if (x._deque == y._deque)
        {
            return (x.absPosition() >= y.absPosition());
        }
        return false;
    }

    pointer get() const;
    void move(difference_type n);
    difference_type absPosition() const;

    deque_const_iterator(container_type const* const theDeque, size_type const current)
    : _deque(theDeque), _current(current)
    {}

    container_type const* _deque = nullptr;
    size_type _current           = 0U;
};

/**
 * Returns a new iterator that is 'n' places ahead of 'other'.
 */
template<class T>
deque_const_iterator<T> operator+(
    typename deque_const_iterator<T>::difference_type n, deque_const_iterator<T> const& other);

/*
 *
 *
 * Implementation
 *
 *
 */

template<class T>
inline bool operator==(deque<T> const& lhs, deque<T> const& rhs)
{
    return (lhs.size() == rhs.size())
           && (std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin()) == true);
}

template<class T>
inline bool operator!=(deque<T> const& lhs, deque<T> const& rhs)
{
    return !(lhs == rhs);
}

template<class T>
inline bool operator<(deque<T> const& lhs, deque<T> const& rhs)
{
    return (lhs.size() == rhs.size())
           && (std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end())
               == true);
}

template<class T>
inline bool operator>(deque<T> const& lhs, deque<T> const& rhs)
{
    return (rhs < lhs);
}

template<class T>
inline bool operator>=(deque<T> const& lhs, deque<T> const& rhs)
{
    return !(rhs > lhs);
}

template<class T>
inline bool operator<=(deque<T> const& lhs, deque<T> const& rhs)
{
    return !(lhs > rhs);
}

/*
 *
 * deque<T> class implementation
 *
 */

// protected
template<class T>
deque<T>::deque(uint8_t* const data, size_type const N)
: _data(data), _num_elements(N), _head(0U), _tail(static_cast<size_type>(N - 1U))
{}

// protected
template<class T>
deque<T>& deque<T>::operator=(deque<T> const& other)
{
    // same data
    if (_data == other._data)
    {
        return *this;
    }

    auto const thisSize = static_cast<difference_type>(size());

    // verify that we have enough space to copy the elements.
    estd_assert(max_size() >= other.size());

    if (size() >= other.size())
    {
        (void)std::copy(other.begin(), other.end(), begin());
        (void)erase(cbegin() + static_cast<difference_type>(other.size()), cend());
    }
    else
    {
        (void)std::copy(other.begin(), other.begin() + thisSize, begin());
        (void)insert(cbegin() + thisSize, other.begin() + thisSize, other.end());
    }

    return *this;
}

template<class T>
inline void deque<T>::assign(size_type n, const_reference value)
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
inline typename deque<T>::iterator deque<T>::begin()
{
    return iterator(this, _head);
}

template<class T>
inline typename deque<T>::const_iterator deque<T>::begin() const
{
    return const_iterator(this, _head);
}

template<class T>
inline typename deque<T>::const_iterator deque<T>::cbegin() const
{
    return begin();
}

template<class T>
inline typename deque<T>::iterator deque<T>::end()
{
    size_type const current = (_tail + 1U) % _num_elements;
    return iterator(this, current);
}

template<class T>
inline typename deque<T>::const_iterator deque<T>::end() const
{
    auto const current = static_cast<size_type>((_tail + 1U) % _num_elements);
    return const_iterator(this, current);
}

template<class T>
inline typename deque<T>::const_iterator deque<T>::cend() const
{
    return end();
}

template<class T>
inline typename deque<T>::reverse_iterator deque<T>::rbegin()
{
    return reverse_iterator(end());
}

template<class T>
inline typename deque<T>::const_reverse_iterator deque<T>::rbegin() const
{
    return const_reverse_iterator(end());
}

template<class T>
inline typename deque<T>::const_reverse_iterator deque<T>::crbegin() const
{
    return const_reverse_iterator(end());
}

template<class T>
inline typename deque<T>::reverse_iterator deque<T>::rend()
{
    return reverse_iterator(begin());
}

template<class T>
inline typename deque<T>::const_reverse_iterator deque<T>::rend() const
{
    return const_reverse_iterator(begin());
}

template<class T>
inline typename deque<T>::const_reverse_iterator deque<T>::crend() const
{
    return const_reverse_iterator(begin());
}

template<class T>
inline typename deque<T>::size_type deque<T>::size() const
{
    return static_cast<size_type>((((_num_elements + _tail) - _head) + 1U) % _num_elements);
}

template<class T>
inline typename deque<T>::size_type deque<T>::max_size() const
{
    return static_cast<size_type>(_num_elements - 1U);
}

template<class T>
inline bool deque<T>::empty() const
{
    return (0U == size());
}

template<class T>
inline bool deque<T>::full() const
{
    return (size() == max_size());
}

template<class T>
inline void deque<T>::clear()
{
    callDestructors(cbegin(), cend());

    _tail = max_size();
    _head = 0U;
}

template<class T>
void deque<T>::push_back(const_reference value)
{
    estd_assert(!full());
    _tail = (_tail + 1U) % _num_elements;
    (void)constructor<T>(_data, _tail).construct(value);
}

template<class T>
typename deque<T>::reference deque<T>::push_back()
{
    estd_assert(!full());
    _tail = (_tail + 1U) % _num_elements;
    return constructor<T>(_data, _tail).construct();
}

template<class T>
constructor<T> deque<T>::emplace_back()
{
    estd_assert(!full());
    _tail = (_tail + 1U) % _num_elements;
    return constructor<T>(_data, _tail);
}

template<class T>
inline void deque<T>::pop_back()
{
    if (empty())
    {
        return;
    }
    type_utils<T>::destroy(_data, _tail);
    _tail = ((_num_elements + _tail) - 1U) % _num_elements;
}

template<class T>
inline void deque<T>::pop_front()
{
    if (empty())
    {
        return;
    }
    type_utils<T>::destroy(_data, _head);
    _head = (_head + 1U) % _num_elements;
}

template<class T>
inline typename deque<T>::const_reference deque<T>::back() const
{
    estd_assert(!empty());
    return *type_utils<T>::cast_const_to_type(_data, _tail);
}

template<class T>
inline typename deque<T>::reference deque<T>::back()
{
    estd_assert(!empty());
    return *type_utils<T>::cast_to_type(_data, _tail);
}

template<class T>
inline void deque<T>::push_front(const_reference value)
{
    estd_assert(!full());
    _head = ((_num_elements + _head) - 1U) % _num_elements;
    (void)constructor<T>(_data, _head).construct(value);
}

template<class T>
typename deque<T>::reference deque<T>::push_front()
{
    estd_assert(!full());
    _head = ((_num_elements + _head) - 1U) % _num_elements;
    return constructor<T>(_data, _head).construct();
}

template<class T>
constructor<T> deque<T>::emplace_front()
{
    estd_assert(!full());
    _head = ((_num_elements + _head) - 1U) % _num_elements;
    return constructor<T>(_data, _head);
}

template<class T>
inline typename deque<T>::const_reference deque<T>::front() const
{
    estd_assert(!empty());
    return *type_utils<T>::cast_const_to_type(_data, _head);
}

template<class T>
inline typename deque<T>::reference deque<T>::front()
{
    estd_assert(!empty());
    return *type_utils<T>::cast_to_type(_data, _head);
}

template<class T>
inline typename deque<T>::const_reference deque<T>::operator[](size_type const n) const
{
    auto const idx = static_cast<size_type>((_head + n) % _num_elements);
    return *type_utils<T>::cast_const_to_type(_data, idx);
}

template<class T>
inline typename deque<T>::reference deque<T>::operator[](size_type const n)
{
    size_type const idx = (_head + n) % _num_elements;
    return *type_utils<T>::cast_to_type(_data, idx);
}

template<class T>
inline typename deque<T>::const_reference deque<T>::at(size_type const n) const
{
    estd_assert(n < size());
    return this->operator[](n);
}

template<class T>
inline typename deque<T>::reference deque<T>::at(size_type const n)
{
    estd_assert(n < size());
    return this->operator[](n);
}

template<class T>
inline void deque<T>::resize(size_type const n)
{
    auto const new_size = min(n, max_size());

    if (size() > new_size)
    {
        while (size() > new_size)
        {
            pop_back();
        }
    }
    else
    {
        while (size() < new_size)
        {
            (void)emplace_back().construct();
        }
    }
}

template<class T>
inline void deque<T>::resize(size_type const n, const_reference value)
{
    size_type const new_size = min(n, max_size());

    if (size() > new_size)
    {
        while (size() > new_size)
        {
            pop_back();
        }
    }
    else
    {
        while (size() < new_size)
        {
            (void)emplace_back().construct(value);
        }
    }
}

template<class T>
typename deque<T>::iterator deque<T>::erase(const_iterator const position)
{
    return erase(position, position + 1);
}

template<class T>
typename deque<T>::iterator deque<T>::erase(const_iterator const first, const_iterator const last)
{
    if ((!empty()) && (first != cend()) && (first != last))
    {
        if (first._current <= _tail)
        { // deque payload to be moved is in contiguous memory, only one move required
            auto const dst = first._current;
            auto const src = last._current;
            auto const n   = static_cast<size_type>((_tail - src) + 1UL) % _num_elements;
            moveLeft(dst, src, n);
            _tail
                = ((_num_elements + _tail) - static_cast<size_type>(last - first)) % _num_elements;
            return iterator(this, first._current);
        }
        if (last._current > _head)
        { // deque payload to be moved is in contiguous memory, only one move required
            auto const src = _head;
            auto const dst = _head + static_cast<size_type>(last - first);
            auto const n   = static_cast<size_type>(first._current - src);
            moveRight(dst, src, n);
            _head = (_num_elements + _head + static_cast<size_type>(last - first)) % _num_elements;
            return iterator(this, last._current);
        }

        // deque payload is wrapped
        size_type dst = 0U;
        size_type src = last._current;
        size_type n   = (_tail - src) + 1U;
        moveLeft(dst, src, n);
        _tail = ((_num_elements + _tail) - (src - dst)) % _num_elements;
        src   = _head;
        dst   = static_cast<size_type>(_head + (_num_elements - first._current));
        n     = first._current - src;
        moveRight(dst, src, n);
        _head = (_num_elements + _head + (dst - src)) % _num_elements;
        return iterator(this, 0U);
    }

    return end();
}

template<class T>
constructor<T> deque<T>::emplace(const_iterator position)
{
    estd_assert(!full());

    if (position == cend())
    {
        return emplace_back();
    }
    if (canMoveHeadLeft(position._current))
    { // head can be moved left
        size_type const dst = _head - 1U;
        size_type const src = _head;
        size_type const n   = position._current - _head;
        moveLeft(dst, src, n);
        position--;
        --_head;
        return constructor<T>(_data, position._current);
    }

    // tail can be moved right
    size_type const dst = position._current + 1U;
    size_type const src = position._current;
    size_type const n   = (_tail - position._current) + 1U;
    moveRight(dst, src, n);
    ++_tail;
    return constructor<T>(_data, position._current);
}

// private
template<class T>
bool deque<T>::canMoveHeadLeft(size_type const current) const
{
    if (_head > 0U)
    {
        if (_head < _tail)
        {
            return true;
        }
        if (current >= _head)
        {
            return true;
        }
    }

    return false;
}

// private
template<class T>
inline void deque<T>::moveLeft(size_type const destPos, size_type const srcPos, size_type const n)
{
    if (srcPos == destPos)
    {
        return; // nothing to do
    }
    auto const first   = iterator(this, srcPos);
    auto const last    = first + static_cast<difference_type>(n);
    auto const d_first = iterator(this, destPos);
    auto const d_last  = iterator(this, destPos) + static_cast<difference_type>(n);
    // would cause undefined behaviour in std::move
    estd_assert((d_first < first) || (last <= d_first));

    (void)std::move(first, last, d_first);
    callDestructors(std::max(d_last, first), last); // destroy leftover elements on the right
}

// private
template<class T>
inline void deque<T>::moveRight(size_type const destPos, size_type const srcPos, size_type const n)
{
    if (srcPos == destPos)
    {
        return; // nothing to do
    }
    auto const first   = iterator(this, srcPos);
    auto const last    = first + static_cast<difference_type>(n);
    auto const d_first = iterator(this, destPos);
    auto const d_last  = iterator(this, destPos) + static_cast<difference_type>(n);
    // would cause undefined behaviour in std::move_backward
    estd_assert((d_last <= first) || (last < d_last));

    (void)std::move_backward(first, last, d_last);
    callDestructors(first, std::min(last, d_first)); // destroy leftover elements on the left
}

template<class T>
typename deque<T>::iterator deque<T>::insert(const_iterator const position, const_reference value)
{
    if (full())
    {
        return end();
    }
    if (empty() || (position == cend()))
    {
        (void)emplace_back().construct(value);
        return end() - 1;
    }
    if (canMoveHeadLeft(position._current) == true)
    { // head can be moved left
        size_type const dst = _head - 1U;
        size_type const src = _head;
        size_type const n   = position._current - _head;
        moveLeft(dst, src, n);
        --_head;
        iterator const tmp(this, (position - 1)._current);
        *tmp = value;
        return tmp;
    }

    // tail can be moved right
    auto const dst = static_cast<size_type>(position._current + 1UL);
    auto const src = position._current;
    auto const n   = static_cast<size_type>((_tail - position._current) + 1UL);
    moveRight(dst, src, n);
    ++_tail;
    iterator const tmp(this, position._current);
    *tmp = value;
    return tmp;
}

template<class T>
void deque<T>::insert(const_iterator const position, size_type n, const_reference value)
{
    while ((!full()) && (n > 0U))
    {
        (void)deque::insert(position, value);
        --n;
    }
}

// private
// static
template<class T>
void deque<T>::callDestructors(const_iterator const b, const_iterator const e)
{
    for (const_iterator itr = b; itr != e; ++itr)
    {
        (*itr).~T();
    }
}

/*
 *
 * class deque_iterator
 *
 */
template<class T>
inline typename deque_iterator<T>::reference deque_iterator<T>::operator*() const
{
    return *get();
}

template<class T>
inline typename deque_iterator<T>::pointer deque_iterator<T>::operator->() const
{
    return get();
}

template<class T>
inline deque_iterator<T>& deque_iterator<T>::operator++()
{
    move(1);
    return *this;
}

template<class T>
inline deque_iterator<T> deque_iterator<T>::operator++(int)
{
    deque_iterator const tmp(*this);
    move(1);
    return tmp;
}

template<class T>
inline deque_iterator<T>& deque_iterator<T>::operator--()
{
    move(-1);
    return *this;
}

template<class T>
inline deque_iterator<T> deque_iterator<T>::operator--(int)
{
    deque_iterator const tmp(*this);
    move(-1);
    return tmp;
}

template<class T>
inline deque_iterator<T>& deque_iterator<T>::operator+=(difference_type const n)
{
    move(n);
    return *this;
}

template<class T>
inline deque_iterator<T>& deque_iterator<T>::operator-=(difference_type const n)
{
    move(static_cast<difference_type>(-n));
    return *this;
}

template<class T>
inline deque_iterator<T> deque_iterator<T>::operator+(difference_type const n) const
{
    deque_iterator tmp(*this);
    tmp += n;
    return tmp;
}

template<class T>
inline deque_iterator<T> deque_iterator<T>::operator-(difference_type const n) const
{
    deque_iterator tmp(*this);
    tmp -= n;
    return tmp;
}

// Difference operator does not have an assignment version
template<class T>
inline typename deque_iterator<T>::difference_type
deque_iterator<T>::operator-(deque_iterator const& other) const
{
    if ((_deque != nullptr) && (_deque == other._deque))
    {
        auto const a1 = absPosition();
        auto const a2 = other.absPosition();
        if (a1 > a2)
        {
            return static_cast<difference_type>(a1 - a2);
        }

        return static_cast<difference_type>(a2 - a1);
    }
    return 0;
}

template<class T>
inline typename deque_iterator<T>::reference
deque_iterator<T>::operator[](difference_type const n) const
{
    return *(this->operator+(n));
}

// private
template<class T>
inline typename deque_iterator<T>::pointer deque_iterator<T>::get() const
{
    if (_deque == nullptr)
    {
        return nullptr;
    }
    return type_utils<T>::cast_to_type(_deque->_data, _current);
}

// private
template<class T>
inline void deque_iterator<T>::move(difference_type const n)
{
    if (_deque != nullptr)
    {
        difference_type const newPos = (static_cast<difference_type>(_deque->_num_elements)
                                        + static_cast<difference_type>(_current) + n)
                                       % static_cast<difference_type>(_deque->_num_elements);

        _current = static_cast<size_type>(newPos);
    }
}

// private
template<class T>
inline typename deque_iterator<T>::difference_type deque_iterator<T>::absPosition() const
{
    if (_deque == nullptr)
    {
        return 0;
    }

    difference_type const safeTypeCast = (static_cast<difference_type>(_current)
                                          + static_cast<difference_type>(_deque->_num_elements))
                                         - static_cast<difference_type>(_deque->_head);

    return static_cast<difference_type>(
        safeTypeCast % static_cast<difference_type>(_deque->_num_elements));
}

template<class T>
inline deque_iterator<T>
operator+(typename deque_iterator<T>::difference_type const n, deque_iterator<T>& other)
{
    other += n;
    return other;
}

/*
 *
 * deque_const_iterator
 *
 */

template<class T>
inline typename deque_const_iterator<T>::reference deque_const_iterator<T>::operator*() const
{
    return *get();
}

template<class T>
inline typename deque_const_iterator<T>::pointer deque_const_iterator<T>::operator->() const
{
    return get();
}

template<class T>
inline deque_const_iterator<T>& deque_const_iterator<T>::operator++()
{
    move(1);
    return *this;
}

template<class T>
inline deque_const_iterator<T> deque_const_iterator<T>::operator++(int)
{
    deque_const_iterator const tmp(*this);
    move(1);
    return tmp;
}

template<class T>
inline deque_const_iterator<T>& deque_const_iterator<T>::operator--()
{
    move(-1);
    return *this;
}

template<class T>
inline deque_const_iterator<T> deque_const_iterator<T>::operator--(int)
{
    deque_const_iterator const tmp(*this);
    move(-1);
    return tmp;
}

template<class T>
inline deque_const_iterator<T>& deque_const_iterator<T>::operator+=(difference_type const n)
{
    move(n);
    return *this;
}

template<class T>
inline deque_const_iterator<T>& deque_const_iterator<T>::operator-=(difference_type const n)
{
    move(static_cast<difference_type>(-n));
    return *this;
}

template<class T>
inline deque_const_iterator<T> deque_const_iterator<T>::operator+(difference_type const n) const
{
    deque_const_iterator tmp(*this);
    tmp += n;
    return tmp;
}

template<class T>
inline deque_const_iterator<T> deque_const_iterator<T>::operator-(difference_type const n) const
{
    deque_const_iterator tmp(*this);
    tmp -= n;
    return tmp;
}

// Difference operator does not have an assignment version
template<class T>
inline typename deque_const_iterator<T>::difference_type
deque_const_iterator<T>::operator-(deque_const_iterator const& other) const
{
    if ((_deque != nullptr) && (_deque == other._deque))
    {
        auto const a1 = absPosition();
        auto const a2 = other.absPosition();
        if (a1 > a2)
        {
            return static_cast<difference_type>(a1 - a2);
        }

        return static_cast<difference_type>(a2 - a1);
    }

    return 0;
}

template<class T>
inline typename deque_const_iterator<T>::reference
deque_const_iterator<T>::operator[](difference_type const n) const
{
    return *(this->operator+(n));
}

// private
template<class T>
inline typename deque_const_iterator<T>::pointer deque_const_iterator<T>::get() const
{
    if (_deque == nullptr)
    {
        return nullptr;
    }

    return type_utils<T>::cast_const_to_type(_deque->_data, _current);
}

// private
template<class T>
inline void deque_const_iterator<T>::move(difference_type const n)
{
    if (_deque != nullptr)
    {
        auto const currentPos = _deque->_num_elements + _current;
        auto const movePos    = static_cast<difference_type>(currentPos) + n;
        auto const newPos     = static_cast<size_type>(movePos);

        _current = static_cast<size_type>(newPos % _deque->_num_elements);
    }
}

// private
template<class T>
inline typename deque_const_iterator<T>::difference_type
deque_const_iterator<T>::absPosition() const
{
    if (_deque == nullptr)
    {
        return 0;
    }
    auto const currPos = static_cast<size_type>(
        ((_current + _deque->_num_elements) - _deque->_head) % _deque->_num_elements);
    return static_cast<difference_type>(currPos);
}

template<class T>
inline deque_const_iterator<T>
operator+(typename deque_const_iterator<T>::difference_type const n, deque_const_iterator<T>& other)
{
    other += n;
    return other;
}

/*
 *
 * class declare::deque
 */
namespace declare
{
template<class T, std::size_t N>
deque<T, N>::deque() : base(&_realData[0], N + 1U)
{}

template<class T, std::size_t N>
deque<T, N>::deque(size_type const n) : base(&_realData[0], N + 1U)
{
    auto const value = value_type();
    auto const total = min(n, N);
    for (size_type i = 0U; i < total; ++i)
    {
        (void)base::emplace_back().construct(value);
    }
}

template<class T, std::size_t N>
deque<T, N>::deque(size_type const n, const_reference value) : base(&_realData[0], N + 1U)
{
    auto const total = min(n, N);
    for (size_type i = 0U; i < total; ++i)
    {
        (void)base::emplace_back().construct(value);
    }
}

template<class T, std::size_t N>
deque<T, N>::deque(base const& other) : base(&_realData[0], N + 1U)
{
    estd_assert(N >= other.max_size());
    for (const_iterator iter = other.cbegin(); iter != other.cend(); ++iter)
    {
        (void)base::emplace_back().construct(*iter);
    }
}

template<class T, std::size_t N>
deque<T, N>::deque(this_type const& other) : base(&_realData[0], N + 1U)
{
    for (const_iterator iter = other.cbegin(); iter != other.cend(); ++iter)
    {
        (void)base::emplace_back().construct(*iter);
    }
}

template<class T, std::size_t N>
deque<T, N>::~deque()
{
    base::clear();
}

template<class T, std::size_t N>
deque<T, N>& deque<T, N>::operator=(base const& other)
{
    base::operator=(other);
    return *this;
}

template<class T, std::size_t N>
deque<T, N>& deque<T, N>::operator=(this_type const& other)
{
    base::operator=(other);
    return *this;
}

} // namespace declare
} // namespace estd
