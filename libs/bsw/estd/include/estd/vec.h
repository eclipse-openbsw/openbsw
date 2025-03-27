// Copyright 2024 Accenture.

#pragma once

#include "estd/algorithm.h"
#include "estd/assert.h"
#include "estd/slice.h"
#include "estd/type_traits.h"

#include <platform/estdint.h>

#include <iterator>
#include <new>
#include <utility>

namespace estd
{

/**
 * A fixed-sized lightweight STL-like vector with internal memory management.
 *
 * [TPARAMS_START]
 * \tparam  T   Type of elements of this vec.
 * \tparam  N   Maximum number of elements this vec can hold.
 * [TPARAMS_END]
 *
 * In contrast to STL container classes vec doesn't allocate on the heap but provides internal
 * storage of sizeof(T) * N contiguous bytes (+ padding) to be able to allocate a maximum of
 * N elements in it.
 *
 * This class does NOT replace estd::vector nor is it "vector version 2.0". It's
 * simply a more lightweight version for some use cases, where estd::vector produced too much
 * overhead, e.g. to represent the payload of a frame.
 *
 * A vec<T. N> doesn't have a common base class vec<T> like estd::vector. This means the
 * capacity of the vec is always part of the type. When passing a vec to a function
 * that operates on the data it is most frequently converted to an estd::slice<T>.
 *
 * \see estd::vector
 * \see estd::slice
 */
template<class T, size_t N>
class vec final
{
public:
    using this_type = ::estd::vec<T, N>;

    // [PUBLIC_TYPES_START]
    /** The template parameter T, i.e. type of elements of this vec. */
    using value_type             = T;
    /** A reference to the template parameter T */
    using reference              = T&;
    /** A const reference to the template parameter T */
    using const_reference        = T const&;
    /** A pointer to the template parameter T */
    using pointer                = T*;
    /** A pointer to the const template parameter T */
    using const_pointer          = T const*;
    /** An unsigned integral type for the size */
    using size_type              = size_t;
    /** A signed integral type for identifying the distance between iterators of this vec */
    using difference_type        = std::ptrdiff_t;
    /** An iterator over this vec */
    using iterator               = pointer;
    /** A const iterator over this vec */
    using const_iterator         = const_pointer;
    /** A reverse iterator over this vec */
    using reverse_iterator       = std::reverse_iterator<iterator>;
    /** A const reverse iterator over this vec */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    // [PUBLIC_TYPES_END]

    // [PUBLIC_API_START]

    /** Constant for the maximum number of elements this vec can hold */
    static constexpr size_t max_size = N;

    /**
     * Default constructor constructing an empty vec.
     */
    vec() = default;

    /**
     * Copy constructor.
     */
    vec(vec const& other);

    /**
     * Copy assignment operator.
     */
    vec& operator=(vec const& other);

    /**
     * Constructs a vec of n copies of a given value.
     * \param n     Number of elements to construct
     * \param value Value for the constructed elements.
     *
     * If n > N, only N elements will be constructed.
     *
     * \warning
     * As vec also provides a std::initializer_list<T> constructor,
     * there are cases where you can't use braced initializer list
     * syntax to call this constructor. The rule is pretty simple:
     * if the compiler can use an initializer list constructor with
     * a brace-initialized list, then it will.
     */
    vec(size_type n, const_reference value);

    /**
     * Constructs a vec with n default-constructed values of T.
     * \param n Number of elements to construct
     *
     * If n > N, only N elements will be constructed.
     */
    explicit vec(size_type n);

    /**
     * Constructs a vec from a given slice of type T.
     * \param init slice to copy from.
     *
     * If init.size() > N, only N elements will be copied.
     */
    explicit vec(slice<T const> const& init);

    /**
     * Destroys this vec and calls the destructor of all elements.
     */
    ~vec();

    /**
     * Returns the current number of elements in this vec.
     */
    size_type size() const;

    /**
     * Returns true if this vec has zero elements, false otherwise.
     */
    bool empty() const;

    /**
     * Returns true if no more elements can be inserted into this vec.
     */
    bool full() const;

    /**
     * Returns the remaining number of elements that can be inserted into this vec.
     */
    size_type free() const;

    /** Returns an iterator to the beginning. */
    iterator begin();

    /** Returns a const iterator to the beginning. */
    const_iterator begin() const;

    /** Returns a const iterator to the beginning. */
    const_iterator cbegin() const;

    /** Returns a reverse iterator to the beginning. */
    reverse_iterator rbegin();

    /** Returns a const reverse iterator to the beginning. */
    const_reverse_iterator rbegin() const;

    /** Returns a const reverse iterator to the beginning. */
    const_reverse_iterator crbegin() const;

    /** Returns an iterator to the end. */
    iterator end();

    /** Returns a const iterator to the end. */
    const_iterator end() const;

    /** Returns a const iterator to the end. */
    const_iterator cend() const;

    /** Returns a reverse iterator to the end. */
    reverse_iterator rend();

    /** Returns a const reverse iterator to the end. */
    const_reverse_iterator rend() const;

    /** Returns a const reverse iterator to the end. */
    const_reverse_iterator crend() const;

    /** Write access to a specific element at index n. */
    reference operator[](size_type n);

    /** Read access to a specific element at index n. */
    const_reference operator[](size_type n) const;

    /**
     * Write access to a specific element at index n.
     * \assert  n < size()
     */
    reference at(size_type n);

    /**
     * Read access to a specific element at index n.
     * \assert  n < size()
     */
    const_reference at(size_type n) const;

    /**
     * Write access to the first element.
     * \assert  size() > 0
     */
    reference front();

    /**
     * Read access to the first element.
     * \assert  size() > 0
     */
    const_reference front() const;

    /**
     * Write access to the last element.
     * \assert  size() > 0
     */
    reference back();

    /**
     * Read access to the last element.
     * \assert  size() > 0
     */
    const_reference back() const;

    /**
     * Returns a pointer to the first element of the internal data of this vec.
     *
     * This function is here to allow conversions to estd::slice<T>.
     *
     * \see estd::slice
     */
    pointer data();

    /**
     * Returns a const_pointer to the first element of the internal data of this vec.
     *
     * This function is here to allow conversions to estd::slice<T>.
     *
     * \see estd::slice
     */
    const_pointer data() const;

    /**
     * Default constructs an element at the end of the vec and returns a reference to it.
     * The size of this vec will be increased by one.
     * \assert  !full()
     */
    reference push_back();

    /**
     * Copy constructs an element at the end of the vec from a given value.
     * The size of this vec will be increased by one.
     * \param value Value to copy construct from
     * \assert  !full()
     */
    void push_back(const_reference value);

    /**
     * Removes the last element of this vec, destroying it.
     * The size of this vec will be decreased by one.
     * \assert  !empty()
     */
    void pop_back();

    /**
     * Constructs an element at the end of this vec and returns a reference to it.
     * \tparam Args Parameter pack of T's constructor arguments
     * \param args  T's constructor parameters
     * \assert  !full()
     */
    template<typename... Args>
    reference emplace_back(Args&&... args);

    /**
     * Clears the vec and adds n copies of a given value to it.
     * \param n Number of copies to add and size after this call
     * \param value Value to fill the vec with
     *
     * If n > N, only N elements will be constructed.
     */
    void assign(size_type n, const_reference value);

    /**
     * Clears the vec and adds copies from an input iterator range to it.
     * \tparam InputIterator    Type of input iterator
     * \param first Beginning of input range
     * \param last End of input range
     *
     * After clearing the vec, elements of the range [first, last) are copied
     * into the vec. If the range is larger than the capacity of this vec only
     * N elements will be copied.
     */
    template<class InputIterator>
    typename std::enable_if<std::is_integral<InputIterator>::value == false, void>::type
    assign(InputIterator first, InputIterator last);

    /**
     * Resizes this vec to a size of n.
     * \param n Size of this vec after the call.
     *
     * If n > N, only N will be used as value for n.
     * If n > size(), default constructed elements will be inserted at the end.
     * If n < size(), the surplus elements will be destroyed.
     */
    void resize(size_type n);

    /**
     * Clears this vec calling the destructor of all elements and setting the size to zero.
     */
    void clear();

    // [PUBLIC_API_END]
private:
    size_type _size = 0U;
    estd::aligned_mem<T> _data[N];
};

// static
template<class T, size_t N>
constexpr size_t vec<T, N>::max_size;

template<class T, size_t N>
inline vec<T, N>::vec(size_type const n, const_reference value)
{
    // n will be limited to N if it's too big in assign.
    assign(n, value);
}

template<class T, size_t N>
vec<T, N>::vec(vec<T, N> const& other)
{
    for (_size = 0U; _size < other.size(); ++_size)
    {
        (void)new (&_data[_size]) T(other[_size]);
    }
}

template<class T, size_t N>
vec<T, N>& vec<T, N>::operator=(vec<T, N> const& other)
{
    if (&other != this)
    {
        assign(other.begin(), other.end());
    }
    return *this;
}

template<class T, size_t N>
inline vec<T, N>::vec(size_type n)
{
    // This constructor doesn't call assign(n, T()) because this would copy construct the elements.
    n = estd::min(n, N);
    for (_size = 0U; _size < n; ++_size)
    {
        (void)new (&_data[_size]) T();
    }
}

template<class T, size_t N>
inline vec<T, N>::vec(slice<T const> const& init)
{
    assign(init.begin(), init.end());
}

template<class T, size_t N>
inline vec<T, N>::~vec()
{
    clear();
}

template<class T, size_t N>
inline typename vec<T, N>::size_type vec<T, N>::size() const
{
    return _size;
}

template<class T, size_t N>
inline bool vec<T, N>::empty() const
{
    return _size == 0U;
}

template<class T, size_t N>
inline bool vec<T, N>::full() const
{
    return _size == N;
}

template<class T, size_t N>
inline typename vec<T, N>::size_type vec<T, N>::free() const
{
    return max_size - _size;
}

template<class T, size_t N>
inline typename vec<T, N>::iterator vec<T, N>::begin()
{
    return _data[0].cast_to_type();
}

template<class T, size_t N>
inline typename vec<T, N>::const_iterator vec<T, N>::begin() const
{
    return _data[0].cast_const_to_type();
}

template<class T, size_t N>
inline typename vec<T, N>::const_iterator vec<T, N>::cbegin() const
{
    return begin();
}

template<class T, size_t N>
inline typename vec<T, N>::reverse_iterator vec<T, N>::rbegin()
{
    return reverse_iterator(end());
}

template<class T, size_t N>
inline typename vec<T, N>::const_reverse_iterator vec<T, N>::rbegin() const
{
    return const_reverse_iterator(end());
}

template<class T, size_t N>
inline typename vec<T, N>::const_reverse_iterator vec<T, N>::crbegin() const
{
    return const_reverse_iterator(end());
}

template<class T, size_t N>
inline typename vec<T, N>::iterator vec<T, N>::end()
{
    return _data[_size].cast_to_type();
}

template<class T, size_t N>
inline typename vec<T, N>::const_iterator vec<T, N>::end() const
{
    return _data[_size].cast_const_to_type();
}

template<class T, size_t N>
inline typename vec<T, N>::const_iterator vec<T, N>::cend() const
{
    return end();
}

template<class T, size_t N>
inline typename vec<T, N>::reverse_iterator vec<T, N>::rend()
{
    return reverse_iterator(begin());
}

template<class T, size_t N>
inline typename vec<T, N>::const_reverse_iterator vec<T, N>::rend() const
{
    return const_reverse_iterator(begin());
}

template<class T, size_t N>
inline typename vec<T, N>::const_reverse_iterator vec<T, N>::crend() const
{
    return rend();
}

template<class T, size_t N>
inline typename vec<T, N>::reference vec<T, N>::operator[](size_type const n)
{
    return *_data[n].cast_to_type();
}

template<class T, size_t N>
inline typename vec<T, N>::const_reference vec<T, N>::operator[](size_type const n) const
{
    return *_data[n].cast_const_to_type();
}

template<class T, size_t N>
inline typename vec<T, N>::reference vec<T, N>::at(size_type const n)
{
    estd_assert(n < _size);
    return (*this)[n];
}

template<class T, size_t N>
inline typename vec<T, N>::const_reference vec<T, N>::at(size_type const n) const
{
    estd_assert(n < _size);
    return (*this)[n];
}

template<class T, size_t N>
inline typename vec<T, N>::reference vec<T, N>::front()
{
    estd_assert(_size > 0U);
    return *begin();
}

template<class T, size_t N>
inline typename vec<T, N>::const_reference vec<T, N>::front() const
{
    estd_assert(_size > 0U);
    return *begin();
}

template<class T, size_t N>
inline typename vec<T, N>::reference vec<T, N>::back()
{
    return at(_size - 1);
}

template<class T, size_t N>
inline typename vec<T, N>::const_reference vec<T, N>::back() const
{
    return at(_size - 1);
}

template<class T, size_t N>
inline typename vec<T, N>::pointer vec<T, N>::data()
{
    return _data[0].cast_to_type();
}

template<class T, size_t N>
inline typename vec<T, N>::const_pointer vec<T, N>::data() const
{
    return _data[0].cast_const_to_type();
}

template<class T, size_t N>
inline typename vec<T, N>::reference vec<T, N>::push_back()
{
    estd_assert(!full());
    auto& result = *new (&_data[_size]) T();
    ++_size;
    return result;
}

template<class T, size_t N>
inline void vec<T, N>::push_back(const_reference value)
{
    estd_assert(!full());
    (void)new (&_data[_size]) T(value);
    ++_size;
}

template<class T, size_t N>
inline void vec<T, N>::pop_back()
{
    estd_assert(!empty());
    (*this)[_size - 1].~T();
    --_size;
}

template<class T, size_t N>
template<typename... Args>
inline typename vec<T, N>::reference vec<T, N>::emplace_back(Args&&... args)
{
    estd_assert(!full());
    auto& result = *new (&_data[_size]) T(std::forward<Args>(args)...);
    ++_size;
    return result;
}

template<class T, size_t N>
inline void vec<T, N>::assign(size_type const n, const_reference value)
{
    clear();
    _size = estd::min(n, N);
    for (size_type i = 0; i < _size; ++i)
    {
        (void)new (&_data[i]) T(value);
    }
}

template<class T, size_t N>
template<class InputIterator>
inline typename std::enable_if<std::is_integral<InputIterator>::value == false, void>::type
vec<T, N>::assign(InputIterator first, InputIterator const last)
{
    clear();
    for (; (!full()) && (first != last); ++first)
    {
        // We don't call emplace_back here to avoid the extra estd_assert per call.
        // We can dereference first because it's an iterator and not necessarily a pointer. Also,
        // the previous check != last makes it safe to assume that first can be dereferenced.
        (void)new (&_data[_size]) T(*first);
        ++_size;
    }
}

template<class T, size_t N>
inline void vec<T, N>::resize(size_type n)
{
    n = estd::min(n, N);
    for (size_type i = n; i < _size; ++i)
    {
        _data[i].cast_to_type()->~T();
    }
    for (size_type i = _size; i < n; ++i)
    {
        (void)new (&_data[i]) T();
    }
    _size = n;
}

template<class T, size_t N>
inline void vec<T, N>::clear()
{
    for (auto& v : *this)
    {
        v.~T();
    }
    _size = 0U;
}

} // namespace estd
