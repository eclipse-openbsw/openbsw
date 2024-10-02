// Copyright 2024 Accenture.

#ifndef GUARD_77D4C7DE_5249_44EA_8B29_3E0997DCB340
#define GUARD_77D4C7DE_5249_44EA_8B29_3E0997DCB340

#include "estd/assert.h"
#include "estd/iterator.h"

#include <platform/estdint.h>

#include <functional>
#include <iterator>
#include <type_traits>

namespace estd
{
template<class T>
class forward_list_node;
template<class T>
class forward_list_iterator;
template<class T>
class forward_list_const_iterator;

/**
 * A C++11 like std::forward_list data structure, that utilizes intrusive
 * nodes.
 * [TPARAMS_START]
 * \tparam  T   Any type that inherits from forward_list_node.
 * [TPARAMS_END]
 *
 * \see forward_list_node
 */
template<class T>
class forward_list
{
    using node_ = forward_list_node<T>;

#ifndef __ARMCC_VERSION
    static_assert(std::is_base_of<forward_list_node<T>, T>::value, "");
#endif
public:
    forward_list(forward_list const&)            = delete;
    forward_list& operator=(forward_list const&) = delete;

    // [PUBLIC_TYPES_START]
    /** The template parameter T, i.e. type of elements of this list. */
    using value_type      = T;
    /** A reference to the template parameter T */
    using reference       = T&;
    /** A const reference to the template parameter T */
    using const_reference = T const&;
    /** A pointer to the template parameter T */
    using pointer         = T*;
    /** A const pointer to the template parameter T */
    using const_pointer   = T const*;
    /** An unsigned integral type for the size */
    using size_type       = std::size_t;
    /** An iterator over this list */
    using iterator        = forward_list_iterator<value_type>;
    /** A const iterator over this list */
    using const_iterator  = forward_list_const_iterator<value_type>;
    // [PUBLIC_TYPES_END]

    // [PUBLIC_API_START]
    /**
     * Constructs an empty list.
     */
    forward_list();

    /**
     * Constructs a list with all elements from an iterator in it.
     */
    template<class InputIt>
    forward_list(InputIt first, InputIt last);

    /**
     * Marks all nodes that are still in the list as unused.
     */
    ~forward_list();

    /**
     * Inserts a given element at the beginning of the list.
     * Inserts the given element before the current first element of the
     * list, increasing the size of the list by one.
     *
     * \param   value   Element to insert.
     * \warning
     * As this is an intrusive list, value must not be part of another list
     * when being inserted, because this would effectively insert value and
     * all element value's next pointer points to, compromising the list
     * in value is currently part of.
     *
     * \complexity O(1)
     */
    void push_front(forward_list_node<T>& value);

    /**
     * Access to first element.
     *
     * \return A reference to the first element.
     * \assert{!empty()}
     * \complexity O(1).
     */
    reference front();

    /**
     * Access to first element.
     *
     * \return A const reference to the first element
     * \assert{!empty()}
     * \complexity O(1).
     */
    const_reference front() const;

    /**
     * Removes the first element of the list. Does not delete
     * the node.
     */
    void pop_front();

    /**
     * Is the list empty?
     *
     * \return
     * - true   if list is empty
     * - false  otherwise
     */
    bool empty() const;

    /**
     * This list implementation can never be full.
     * \return false
     */
    bool full() const;

    /**
     * The number of elements in the container.
     *
     * \return size of the container
     */
    std::size_t size() const;

    /**
     * Clears content of the list.
     */
    void clear();

    /**
     * Reverses order of the list's elements.
     */
    void reverse();

    /**
     * Removes the specified node if it is a part of this list.
     *
     * \param node The list node to remove.
     */
    void remove(forward_list_node<T>& node);

    /**
     * Removes all elements fulfilling a given condition.
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

    template<class Predicate, class Removed>
    void remove_if(Predicate pred, Removed& removed);

    /**
     * Checks if a given node is contained in this list.
     *
     * \param node The node to find.
     * \return
     * - true if the node is in this list
     * - false otherwise.
     */
    bool contains_node(forward_list_node<T> const& node) const;

    /**
     * Swaps the content of the list with a given other list.
     *
     * \param   other   forward_list to swap content with.
     */
    void swap(forward_list& other);

    /**
     * Transfers all the elements from 'other' to this forward_list,
     * following 'position'.
     *
     * \param position An iterator to where elements are inserted
     * \param other The other list
     *
     * \warning
     * The behavior is undefined in the following cases:
     * - The 'position' iterator is not an iterator to 'this' list.
     * - The 'other' list is the same as 'this' list.
     */
    void splice_after(iterator position, forward_list& other);

    /**
     * Transfers the element after i to the element following 'position'.
     *
     * \param position An iterator to where elements will be inserted
     * \param i An iterator preceding the one to be transferred.
     */
    void splice_after(iterator position, iterator i);

    /**
     * Transfers the element after i to the element following 'position'.
     *
     * \param position An iterator to where elements will be inserted
     * \param other The other list to copy from
     * \param i An iterator preceding the one to be transferred.
     */
    void splice_after(iterator position, forward_list const& other, iterator i);

    /**
     * Transfers the elements (first,last) to this forward_list,
     * following position. The first and last elements are not transferred.
     */
    void splice_after(iterator position, iterator first, iterator last);

    /**
     * Transfers the elements (first,last) from 'other' to this forward_list,
     * following position. The first and last elements are not transferred.
     */
    void splice_after(iterator position, forward_list const& other, iterator first, iterator last);

    /**
     * Extends the list by a given element which is inserted after position.
     *
     * \param   position    Iterator after which value will be inserted.
     * \param   value   Element to be inserted.
     * \return  An iterator to the newly inserted element.
     */
    iterator insert_after(iterator position, forward_list_node<T>& value);

    /**
     * Erase the element just after position. To remove the first element,
     * use erase_after(before_begin()).
     *
     * \param position The starting location in this list
     * \return An iterator to the element just after position.
     */
    iterator erase_after(iterator position);

    /**
     * Erase elements following position until last.
     *
     * \param position The starting location in this list
     * \param last The ending location
     * \return An iterator to last
     */
    iterator erase_after(iterator position, iterator last);

    /**
     * Merges two sorted lists into one. The lists have to be sorted in ascending order.
     * No elements are copied. The container other becomes empty after the operation.
     */
    void merge(forward_list& other);

    /**
     * Merges two sorted lists into one. The lists have to be sorted in ascending order.
     * No elements are copied. The container other becomes empty after the operation.

     * The specified Compare object is used to determine equality.
     */
    template<class Compare>
    void merge(forward_list& other, Compare compare);

    /**
     * Sort the elements of the list.
     * Sorts the elements of this list in NlogN time.  Equivalent
     * elements remain in list order.
     *
     * T is required to support operator==
     */
    void sort();

    /**
     * Sort the elements of the list.
     * Sorts the elements of this list in NlogN time.  Equivalent
     * elements remain in list order.
     *
     * The specified Compare object is used to determine equality.
     */
    template<class Compare>
    void sort(Compare compare);

    /** Returns an iterator to the beginning */
    iterator begin();

    /** Returns a const iterator to the beginning */
    const_iterator begin() const;

    /** Returns a const iterator to the beginning */
    const_iterator cbegin() const;

    /** Returns an iterator to the element just before the beginning */
    iterator before_begin();

    /** Returns a const iterator to the element just before the beginning */
    const_iterator before_begin() const;

    /** Returns a const iterator to the element just before the beginning */
    const_iterator cbefore_begin() const;

    /** Returns an iterator to the end */
    iterator end();

    /** Returns a const iterator to the end */
    const_iterator end() const;

    /** Returns a const iterator to the end */
    const_iterator cend() const;
    // [PUBLIC_API_END]

    // [DEPRECATED_API_START]
    /**
     * \deprecated
     * \see ::estd::push_back
     */
    void push_back(forward_list_node<T>& value);

    /**
     * \deprecated
     * \see ::estd::pop_back
     */
    void pop_back();

    /**
     * \deprecated
     * \see ::estd::back
     */
    reference back();

    /**
     * \deprecated
     * \see ::estd::back
     */
    const_reference back() const;
    // [DEPRECATED_API_END]

private:
    pointer lastItem();
    const_pointer lastItem() const;

    forward_list_node<T> _before_first;
};

// [PUBLIC_FUNC_START]
/**
 * Checks whether the node is already contained in any forward_list
 */
template<class T>
bool is_in_use(forward_list_node<T> const&);

/**
 * Access to last element.
 *
 * \return A reference to the last element
 * \assert{!empty()}
 *
 * \note This method should not be used and will be deprecated in the future.
 *       If you require the functionality, either switch to a different data structure (deque)
 *       or maintain a reference/iterator to the back of the list manually.
 *
 * \complexity O(n).
 */
template<class T>
T& back(forward_list<T>& list);

/**
 * Access to last element.
 *
 * \return A const reference to the last element
 * \assert{!empty()}
 *
 * \note This method should not be used and will be deprecated in the future.
 *       If you require the functionality, either switch to a different data structure (deque)
 *       or maintain a reference/iterator to the back of the list manually.
 *
 * \complexity O(n).
 */
template<class T>
T const& back(forward_list<T> const& list);

/**
 * Inserts a given element at the end of the list.
 * Inserts the given element after the current last element of the
 * list, increasing the size of the list by one.
 *
 * \param   list    List to insert into.
 * \param   node    Element to insert.
 * \warning
 * As this is an intrusive list, value must not be part of another list
 * when being inserted, because this would effectively insert value and
 * all element value's next pointer points to, compromising the list
 * in value is currently part of.
 *
 * \note This method should not be used and will be deprecated in the future.
 *       If you require the functionality, either switch to a different data structure (deque)
 *       or maintain a reference/iterator to the back of the list manually.
 *
 * \complexity O(n)
 */
template<class T, class N>
void push_back(forward_list<T>& list, N& node);

/**
 * Removes the last element of the list. Does not delete
 * the node.
 *
 * \note This method should not be used and will be deprecated in the future.
 *       If you require the functionality, either switch to a different data structure (deque)
 *       or maintain a reference/iterator to the back of the list manually.
 *
 * \complexity O(n)
 */
template<class T>
void pop_back(forward_list<T>& list);

/**
 * Swaps two lists
 */
template<class T>
void swap(forward_list<T>& x, forward_list<T>& y);

// [PUBLIC_FUNC_END]

/**
 * Node element of forward_list.
 * This class uses the CRTP. The forward_list_node is an intrusive node,
 * i.e. the next pointer is part of the object and inserted into by
 * inheriting from this class. This also implies one crucial restriction:
 * Any forward_list_node can only be part of one forward_list, because
 * there's only one next pointer.
 * However, it is possible to be part of two lists, if T differs, see
 * example below. An instance of N can only be in one list of type
 * forward_list<N>, an instance of M can only be part of one list of type
 * forward_list<M> at a time. An instance of X can be part of one
 * forward_list<N> and one forward_list<M> at the same time.
 *
 * \section Example
 * \code{.cpp}
 * struct N : forward_list_node<N> {};
 * struct M : forward_list_node<M> {};
 * struct X : N, M {};
 * \endcode
 */
template<class T>
class forward_list_node
{
public:
    forward_list_node(forward_list_node const&)            = delete;
    forward_list_node& operator=(forward_list_node const&) = delete;

    /**
     * Create an empty forward list. Initializing the next
     * pointer to nullptr.
     */
    forward_list_node();

protected:
    ~forward_list_node() = default;

private:
    template<class>
    friend class forward_list;
    template<class>
    friend class forward_list_iterator;
    template<class>
    friend class forward_list_const_iterator;
    friend bool is_in_use<T>(forward_list_node const&);

    forward_list_node* next();

    forward_list_node const* next() const;

    void set_next(forward_list_node* nextNode);
    void set_free();

    // This next pointer will be set to the constant 1 for nodes that
    // are not part of any list. This makes it possible to check whether a node
    // is free in constant time.
    forward_list_node* _next;
};

/**
 * Iterator for forward_list.
 */
template<class T>
class forward_list_iterator
{
public:
    // [PUBLIC_TYPES_START:forward_list_iterator]
    using iterator_category = ::std::forward_iterator_tag;
    using value_type        = T;
    using reference         = T&;
    using pointer           = T*;
    using difference_type   = ::std::ptrdiff_t;
    // [PUBLIC_TYPES_END:forward_list_iterator]

    // [PUBLIC_API_START:forward_list_iterator]
    /** Creates an uninitialized iterator */
    forward_list_iterator();

    /** Dereferences the underlying object at the current iterator position */
    reference operator*() const;

    /** Dereferences the underlying object at the current iterator position */
    pointer operator->() const;

    /** Moves the iterator forward by one position */
    forward_list_iterator& operator++();

    /** Returns an iterator moved forward by one position */
    forward_list_iterator operator++(int);
    // [PUBLIC_API_END:forward_list_iterator]
private:
    using node_ = ::estd::forward_list_node<T>;

    template<class>
    friend class forward_list;
    template<class>
    friend class forward_list_const_iterator;

    explicit forward_list_iterator(node_* const current) : _current(current) {}

    node_* _current;

public:
    /**
     * Compares two forward_list_iterators and returns true if they
     * point to the same element.
     *
     * \param lhs The left-hand side of the equality.
     * \param rhs The right-hand side of the equality.
     * \return * - true if the iterators pointer to the same element
     * - false otherwise.
     */
    friend inline bool
    operator==(forward_list_iterator const& lhs, forward_list_iterator const& rhs)
    {
        return (lhs._current == rhs._current);
    }

    /**
     * Compares two forward_list_iterators and returns true if they
     * point to different elements.
     *
     * \param lhs The left-hand side of the equality.
     * \param rhs The right-hand side of the equality.
     * \return * - true if the iterators pointer to different elements
     * - false otherwise.
     */
    friend inline bool
    operator!=(forward_list_iterator const& lhs, forward_list_iterator const& rhs)
    {
        return !(lhs == rhs);
    }
};

template<class T>
forward_list_iterator<T>::forward_list_iterator() : forward_list_iterator(nullptr)
{}

template<class T>
typename forward_list_iterator<T>::reference forward_list_iterator<T>::operator*() const
{
    return *static_cast<T*>(_current);
}

template<class T>
typename forward_list_iterator<T>::pointer forward_list_iterator<T>::operator->() const
{
    return static_cast<T*>(_current);
}

template<class T>
forward_list_iterator<T>& forward_list_iterator<T>::operator++()
{
    if (_current != nullptr)
    {
        _current = _current->next();
    }
    return *this;
}

template<class T>
forward_list_iterator<T> forward_list_iterator<T>::operator++(int)
{
    forward_list_iterator const tmp(*this);
    (void)this->operator++();
    return tmp;
}

/**
 * Const iterator for forward_list.
 */
template<class T>
class forward_list_const_iterator
{
public:
    // [PUBLIC_TYPES_START:forward_list_const_iterator]
    using iterator_category = ::std::forward_iterator_tag;
    using value_type        = T const;
    using reference         = T const&;
    using pointer           = T const*;
    using difference_type   = ::std::ptrdiff_t;
    // [PUBLIC_TYPES_END:forward_list_const_iterator]

    // [PUBLIC_API_START:forward_list_const_iterator]
    /** Create an uninitialized iterator */
    forward_list_const_iterator();

    /**
     * A const forward_list iterator is constructable from a non-const version, implicit conversion
     * intended.
     */
    forward_list_const_iterator(::estd::forward_list_iterator<T> const& i);

    /** Dereferences the underlying object at the current iterator position. */
    reference operator*() const;

    /** Dereferences the underlying object at the current iterator position. */
    pointer operator->() const;

    /** Moves the iterator forward by one position. */
    forward_list_const_iterator& operator++();

    /** Returns an iterator moved forward by one position. */
    forward_list_const_iterator operator++(int);
    // [PUBLIC_API_END:forward_list_const_iterator]

private:
    using node_ = ::estd::forward_list_node<T>;
    template<class>
    friend class forward_list;

    friend inline bool
    operator==(forward_list_const_iterator const& x, forward_list_const_iterator const& y)
    {
        return (x._current == y._current);
    }

    friend inline bool
    operator!=(forward_list_const_iterator const& x, forward_list_const_iterator const& y)
    {
        return !(x == y);
    }

    explicit forward_list_const_iterator(node_ const* const current) : _current(current) {}

    node_ const* _current;
};

template<class T>
forward_list_const_iterator<T>::forward_list_const_iterator() : forward_list_const_iterator(nullptr)
{}

template<class T>
forward_list_const_iterator<T>::forward_list_const_iterator(
    ::estd::forward_list_iterator<T> const& i)
: forward_list_const_iterator(i._current)
{}

template<class T>
typename forward_list_const_iterator<T>::reference forward_list_const_iterator<T>::operator*() const
{
    return *static_cast<T const*>(_current);
}

template<class T>
typename forward_list_const_iterator<T>::pointer forward_list_const_iterator<T>::operator->() const
{
    return static_cast<T const*>(_current);
}

template<class T>
forward_list_const_iterator<T>& forward_list_const_iterator<T>::operator++()
{
    if (_current != nullptr)
    {
        _current = _current->next();
    }
    return *this;
}

template<class T>
forward_list_const_iterator<T> forward_list_const_iterator<T>::operator++(int)
{
    forward_list_const_iterator const tmp(*this);
    (void)this->operator++();
    return tmp;
}

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

template<class T>
bool is_in_use(forward_list_node<T> const& node)
{
    return reinterpret_cast<uintptr_t>(node._next) != 1U;
}

template<class T>
inline void swap(forward_list<T>& x, forward_list<T>& y)
{
    x.swap(y);
}

/*
 *
 * class forward_list_node
 *
 */
template<class T>
forward_list_node<T>::forward_list_node() : _next(reinterpret_cast<forward_list_node*>(1U))
{}

template<class T>
inline forward_list_node<T>* forward_list_node<T>::next()
{
    return _next;
}

template<class T>
inline forward_list_node<T> const* forward_list_node<T>::next() const
{
    return _next;
}

template<class T>
inline void forward_list_node<T>::set_next(forward_list_node* const nextNode)
{
    _next = nextNode;
}

template<class T>
inline void forward_list_node<T>::set_free()
{
    _next = reinterpret_cast<forward_list_node*>(1U);
}

/*
 *
 * class forward_list
 *
 */
template<class T>
forward_list<T>::forward_list() : _before_first()
{
    _before_first._next = nullptr;
}

template<class T>
template<class InputIt>
forward_list<T>::forward_list(InputIt const first, InputIt const last) : forward_list()
{
    iterator end_of_list = before_begin();
    for (InputIt current = first; current != last; ++current)
    {
        end_of_list = insert_after(end_of_list, *current);
    }
}

template<class T>
forward_list<T>::~forward_list()
{
    clear();
}

template<class T>
inline void forward_list<T>::push_front(forward_list_node<T>& value)
{
    if (!is_in_use(value))
    {
        value.set_next(_before_first.next());
        _before_first.set_next(&value);
    }
}

template<class T>
inline void forward_list<T>::push_back(forward_list_node<T>& value)
{
    if (!is_in_use(value))
    {
        if (empty())
        {
            push_front(value);
        }
        else
        {
            value.set_next(nullptr);
            static_cast<forward_list_node<T>*>(lastItem())->set_next(&value);
        }
    }
}

template<class T, class N>
void push_back(forward_list<T>& list, N& node)
{
    list.push_back(node);
}

template<class T>
T& back(forward_list<T>& list)
{
    return list.back();
}

template<class T>
T const& back(forward_list<T> const& list)
{
    return list.back();
}

template<class T>
void pop_back(forward_list<T>& list)
{
    list.pop_back();
}

template<class T>
inline typename forward_list<T>::reference forward_list<T>::front()
{
    estd_assert(!empty());
    return *static_cast<pointer>(_before_first.next());
}

template<class T>
inline typename forward_list<T>::const_reference forward_list<T>::front() const
{
    estd_assert(!empty());
    return *static_cast<const_pointer>(_before_first.next());
}

template<class T>
inline typename forward_list<T>::reference forward_list<T>::back()
{
    estd_assert(!empty());
    return *lastItem();
}

template<class T>
inline typename forward_list<T>::const_reference forward_list<T>::back() const
{
    estd_assert(!empty());
    return *lastItem();
}

template<class T>
void forward_list<T>::pop_front()
{
    if (empty())
    {
        return;
    }
    auto const tmp = static_cast<pointer>(_before_first.next());
    _before_first.set_next(_before_first.next()->next());
    tmp->node_::set_free();
}

template<class T>
void forward_list<T>::pop_back()
{
    if (empty())
    {
        return;
    }
    auto previous      = before_begin();
    auto current       = before_begin();
    auto peek          = begin();
    auto const endIter = end();
    while (peek != endIter)
    {
        previous = current;
        current  = peek;
        ++peek;
    }
    current->node_::set_free();
    previous._current->set_next(nullptr);
}

template<class T>
inline bool forward_list<T>::empty() const
{
    return (_before_first.next() == nullptr);
}

template<class T>
inline bool forward_list<T>::full() const
{
    return false;
}

template<class T>
inline std::size_t forward_list<T>::size() const
{
    return static_cast<std::size_t>(std::distance(cbegin(), cend()));
}

template<class T>
inline void forward_list<T>::clear()
{
    iterator itr       = begin();
    auto const endIter = end();
    while (itr != endIter)
    {
        auto const tmp = itr;
        ++itr;
        tmp->node_::set_free();
    }
    _before_first.set_next(nullptr);
}

template<class T>
void forward_list<T>::reverse()
{
    forward_list tmp;
    while (!empty())
    {
        reference current = front();
        pop_front();
        tmp.push_front(current);
    }
    _before_first.set_next(tmp._before_first._next);
    tmp._before_first.set_next(nullptr);
}

template<class T>
inline void forward_list<T>::remove(forward_list_node<T>& node)
{
    if (empty())
    {
        return;
    }

    auto current       = before_begin();
    auto previous      = end();
    auto const endIter = end();
    while ((current != endIter) && (current._current != &node))
    {
        previous = current;
        ++current;
    }

    if (current == endIter)
    { // node isn't in this list!
        return;
    }

    previous._current->set_next(node.next());

    node.set_free();
}

namespace internal
{
struct BlackHole
{
    template<class T>
    void push_front(T&)
    {}
};
} // namespace internal

template<class T>
template<class Predicate>
void forward_list<T>::remove_if(Predicate const pred)
{
    internal::BlackHole removed;
    remove_if(pred, removed);
}

template<class T>
template<class Predicate, class Removed>
void forward_list<T>::remove_if(Predicate pred, Removed& removed)
{
    forward_list tmp;
    iterator inItr = tmp.before_begin();
    while (!empty())
    {
        reference current = front();
        pop_front();
        if (pred(current) == false)
        {
            (void)tmp.insert_after(inItr, current);
            ++inItr;
        }
        else
        {
            removed.push_front(current);
        }
    }
    forward_list::swap(tmp);
}

template<class T>
inline bool forward_list<T>::contains_node(forward_list_node<T> const& node) const
{
    if (!is_in_use(node))
    {
        // if the node isn't part of any list, it certainly
        // won't be in this one.
        return false;
    }

    auto itr           = begin();
    auto const endIter = cend();

    while (itr != endIter)
    {
        if (itr.operator->() == &node)
        {
            return true;
        }
        ++itr;
    }
    return false;
}

template<class T>
inline void forward_list<T>::swap(forward_list& other)
{
    node_* const tmp = _before_first.next();
    _before_first.set_next(other._before_first.next());
    other._before_first.set_next(tmp);
}

template<class T>
inline void forward_list<T>::splice_after(iterator const position, forward_list& other)
{
    estd_assert(this != &other);
    if (!other.empty())
    {
        other.lastItem()->node_::set_next(position._current->next());
        position._current->set_next(&other.front());
        other._before_first.set_next(nullptr);
    }
}

template<class T>
void forward_list<T>::splice_after(
    iterator const position, forward_list const& /*other*/, iterator const i)
{
    splice_after(position, i);
}

template<class T>
void forward_list<T>::splice_after(iterator const position, iterator const i)
{
    if (position != i)
    {
        iterator element = iterator(i);
        ++element;
        if (position != element)
        {
            i._current->set_next(element._current->next());
            node_* const tmp = position._current->next();
            position._current->set_next(element._current);
            element._current->set_next(tmp);
        }
    }
}

template<class T>
void forward_list<T>::splice_after(
    iterator const position,
    forward_list const& /*other*/,
    iterator const first,
    iterator const last)
{
    splice_after(position, first, last);
}

template<class T>
void forward_list<T>::splice_after(
    iterator const position, iterator const first, iterator const last)
{
    if (position != first)
    {
        iterator element = ::estd::next(first);
        if ((position != element) && (last != element))
        {
            // first points to last after this
            first._current->set_next(last._current);

            node_* const tmp = position._current->next();
            position._current->set_next(element._current);
            while (element._current->next() != last._current)
            {
                ++element;
            }
            element._current->set_next(tmp);
        }
    }
}

template<class T>
typename forward_list<T>::iterator
forward_list<T>::insert_after(iterator const position, forward_list_node<T>& value)
{
    iterator const endIter = end();

    if ((position == endIter) || is_in_use(value))
    {
        return endIter;
    }

    value.set_next(position._current->next());
    position._current->set_next(&value);
    return iterator(&value);
}

template<class T>
inline typename forward_list<T>::iterator forward_list<T>::erase_after(iterator const position)
{
    iterator const endIter = end();

    if (position == endIter)
    {
        return endIter;
    }

    node_* const tmp  = position._current;
    node_* const next = tmp->next();
    if (next == nullptr)
    {
        return endIter;
    }

    tmp->set_next(next->next());
    next->set_free();

    return iterator(tmp->next());
}

template<class T>
typename forward_list<T>::iterator
forward_list<T>::erase_after(iterator const position, iterator const last)
{
    if (position != last)
    {
        iterator tmp = ::estd::next(position);
        while (tmp != last)
        {
            iterator const next = ::estd::next(tmp);
            tmp->node_::set_free();
            tmp = next;
        }
        position._current->set_next(tmp._current);
    }
    return iterator(last._current);
}

template<class T>
void forward_list<T>::merge(forward_list& other)
{
    merge(other, std::less<T>());
}

template<class T>
template<class Compare>
void forward_list<T>::merge(forward_list& other, Compare const compare)
{
    auto current       = before_begin();
    auto next          = ::estd::next(current);
    auto const endIter = end();

    // iterate until end of this list or until other is empty
    while ((next != endIter) && (!other.empty()))
    {
        // check if first element of other goes before current->next
        if (compare(*other.begin(), *next) == true)
        {
            node_* const other_before_first = &other._before_first;
            node_* const other_first        = other_before_first->next();
            // remove first element in other
            other_before_first->set_next(other_first->next());
            // move other's former first before current
            other_first->set_next(current._current->next());
            current._current->set_next(other_first);
        }
        ++current;
        next = ::estd::next(current);
    }
    // append rest of other if there is any
    if (!other.empty())
    {
        current._current->set_next(other._before_first.next());
        other._before_first.set_next(nullptr);
    }
}

template<class T>
void forward_list<T>::sort()
{
    sort(std::less<T>());
}

template<class T>
template<class Compare>
void forward_list<T>::sort(Compare const compare)
{
    if (empty())
    { // nothing to sort in an empty list
        return;
    }
    auto list          = begin();
    auto const endIter = end();

    size_type list_size  = 1U;
    size_type num_merges = 0U;

    do
    {
        num_merges    = 0U;
        iterator left = list;
        iterator tail = endIter;
        list          = endIter;
        iterator next;
        while (left != endIter)
        {
            ++num_merges;
            iterator right    = left;
            size_t left_size  = 0U;
            size_t right_size = list_size;
            // cut lists into halves
            while ((right != endIter) && (left_size < list_size))
            {
                ++left_size;
                ++right;
            }
            // run through lists and append
            while ((left_size > 0U) || ((right_size > 0U) && (right != endIter)))
            {
                if (left_size == 0U)
                { // left is empty -> take right
                    next = right;
                    ++right;
                    --right_size;
                }
                else if ((right_size == 0U) || (right == endIter))
                { // right is empty -> take left
                    next = left;
                    ++left;
                    --left_size;
                }
                else if (compare(*right, *left) == true)
                { // right goes before left
                    next = right;
                    ++right;
                    --right_size;
                }
                else
                { // left goes before right
                    next = left;
                    ++left;
                    --left_size;
                }
                if (tail != endIter)
                {
                    tail->node_::set_next(next._current);
                }
                else
                {
                    list = next;
                }
                tail = next;
            }
            // right is now after the just sorted part, start next round here
            left = right;
        }
        // terminate tail
        tail->node_::set_next(nullptr);
        // double list size
        list_size <<= 1U;
    } while (num_merges > 1U);
    _before_first.set_next(list._current);
}

template<class T>
inline typename forward_list<T>::iterator forward_list<T>::begin()
{
    return iterator(_before_first.next());
}

template<class T>
inline typename forward_list<T>::const_iterator forward_list<T>::begin() const
{
    return const_iterator(_before_first.next());
}

template<class T>
inline typename forward_list<T>::const_iterator forward_list<T>::cbegin() const
{
    return begin();
}

template<class T>
inline typename forward_list<T>::iterator forward_list<T>::before_begin()
{
    return iterator(&_before_first);
}

template<class T>
inline typename forward_list<T>::const_iterator forward_list<T>::before_begin() const
{
    return const_iterator(&_before_first);
}

template<class T>
inline typename forward_list<T>::const_iterator forward_list<T>::cbefore_begin() const
{
    return before_begin();
}

template<class T>
inline typename forward_list<T>::iterator forward_list<T>::end()
{
    return iterator(nullptr);
}

template<class T>
inline typename forward_list<T>::const_iterator forward_list<T>::end() const
{
    return const_iterator(nullptr);
}

template<class T>
inline typename forward_list<T>::const_iterator forward_list<T>::cend() const
{
    return const_iterator(nullptr);
}

template<class T>
inline typename forward_list<T>::pointer forward_list<T>::lastItem()
{
    iterator itr = begin();
    while (itr._current->next() != nullptr)
    {
        ++itr;
    }
    return itr.operator->();
}

template<class T>
typename forward_list<T>::const_pointer forward_list<T>::lastItem() const
{
    const_iterator itr = begin();
    while (itr._current->next() != nullptr)
    {
        ++itr;
    }
    return itr.operator->();
}

} // namespace estd

#endif // GUARD_77D4C7DE_5249_44EA_8B29_3E0997DCB340
