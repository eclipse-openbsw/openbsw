// Copyright 2024 Accenture.

/**
 * Contains estd::slice class.
 * \file
 * \ingroup estl_time
 */
#ifndef GUARD_2626BDF2_63F2_448A_B810_5929FA66F5FF
#define GUARD_2626BDF2_63F2_448A_B810_5929FA66F5FF

#include "estd/assert.h"
#include "estd/type_traits.h"

#include <platform/estdint.h>

#include <iterator>
#include <type_traits>

namespace estd
{
// forward declare array for specializations
template<class T, size_t N>
class array;

namespace internal
{

template<class C>
class _has_size
{
    template<class T>
    static constexpr auto has_size_helper(T*) ->
        typename estd::is_same<decltype(std::declval<T>().size()), size_t>::type;

    template<class>
    static constexpr estd::false_type has_size_helper(...);

    using type = decltype(has_size_helper<C>(nullptr));

public:
    static constexpr bool value = type::value;
};

} // namespace internal

/**
 * A wrapper around a pointer providing save access to a region
 * of memory interpreted as a particular data type 'T'.
 * This version of the class has a non-zero size as template argument.
 * It is therefore able to do some range safety checks at compile time, and also does not
 * need any memory to store the size at runtime. It has zero overhead compared
 * to a naked pointer.
 *
 * [TPARAMS_BEGIN]
 * \tparam T    Type of elements of the slice.
 * \tparam Size If > 0, specifies the size of a slice which is known at compile time.
 *              If == 0, this parameter isn't used and slice size is stored at a runtime.
 * [TPARAMS_END]
 *
 * Besides the 'Size=0' here, this is the version for 'Size != 0'.
 */
template<class T, size_t Size = 0>
class slice
{
public:
    /** [PUBLIC_TYPES_BEGIN] */
    /** Template parameter T */
    using value_type             = T;
    /** Unsigned integral type */
    using size_type              = std::size_t;
    /** Signed integral type */
    using difference_type        = std::ptrdiff_t;
    /** Reference to T */
    using reference              = T&;
    /** Const reference to T */
    using const_reference        = T const&;
    /** Pointer to T */
    using pointer                = T*;
    /** Pointer to const T */
    using const_pointer          = T const*;
    /** Random access iterator to value_type */
    using iterator               = pointer;
    /** Random access iterator to const value_type */
    using const_iterator         = const_pointer;
    /** Reverse iterator */
    using reverse_iterator       = std::reverse_iterator<iterator>;
    /** Constant reverse iterator */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    /** [PUBLIC_TYPES_END] */

private:
    T* _data;

    // This constructor does no checking, and is therefore
    // declared private. It needs a dummy parameter to make
    // sure it is not chosen by overload resolution when we
    // really want it to choose a different one.
    constexpr slice(T* const theData, bool const /* dummy parameter */) : _data(theData) {}

    template<class OtherT, size_t OtherSize>
    friend class slice;

public:
    // [PUBLIC_API_BEGIN]

    /**
     * Construct a slice referring to a C array.
     * This constructor will check at compile time
     * that the C array is at least as large as the slice
     * being created
     */
    template<size_t OtherSize>
    constexpr slice(T (&theData)[OtherSize]) : _data(theData)
    {
        static_assert(Size <= OtherSize, "static slice size");
    }

    /**
     * Construct a slice referring to an estd::array.
     * This constructor will check at compile time
     * that the estd::array is at least as large as the slice
     * being created
     */
    template<size_t OtherSize>
    constexpr slice(array<T, OtherSize>& theData) : _data(theData.data())
    {
        static_assert(Size <= OtherSize, "static slice size");
    }

    template<class OtherT, size_t OtherSize>
    constexpr slice(array<OtherT, OtherSize> const& theData) : _data(theData.data())
    {
        static_assert(Size <= OtherSize, "static slice size");
    }

    /**
     * Construct from an other static slice.
     * This constructor will check at compile time
     * that the other slice is at least as large as the slice
     * being created
     */
    template<class OtherT, size_t OtherSize>
    constexpr slice(slice<OtherT, OtherSize> const& other) : _data(other.data())
    {
        static_assert(Size <= OtherSize, "static slice size");
    }

    constexpr slice(slice const&)  = default;
    slice& operator=(slice const&) = default;

    /**
     * Empty destructor to work around a diab bug
     * See: https://esrlabs.atlassian.net/browse/DIAB-6
     */
    ~slice() = default;

    /**
     * Construct a slice referring to the memory given by a pointer.
     * Since this operation is not able to do any range checking,
     * the caller must ensure 'data' points to a valid memory region
     * of appropriate size.
     */
    static slice from_pointer(T* const theData) { return slice(theData, false /* dummy */); }

    /**
     * Return the size of the slice.
     * This is a compile-time constant.
     */
    constexpr size_t size() const { return Size; }

    /**
     * Return the raw pointer to the data.
     */
    T* data() const { return _data; }

    /**
     * Return a reference to the element of the slice at the
     * specified location.
     * This method does range checking at compile time.
     */
    template<size_t Index>
    T& at() const
    {
        static_assert(Index < Size, "static slice size");
        return _data[Index];
    }

    /**
     * Return a reference to the element of the slice at the
     * specified location.
     * This method will assert if the index is out of bounds.
     */
    T& at(size_t const idx) const
    {
        estd_assert(idx < Size);
        return _data[idx];
    }

    /**
     * Return a reference to the element of the slice at the
     * specified location.
     */
    T& operator[](size_t const idx) const { return _data[idx]; }

    /**
     * Create a new, smaller, slice by skipping 'Offset' elements
     * from the beginning of this slice.
     * 'Offset' must be smaller than 'Size'. (this is checked at compile-time)
     */
    template<size_t Offset>
    slice<T, Size - Offset> offset() const
    {
        static_assert(Offset < Size, "static slice size");
        return slice<T, Size - Offset>(&_data[Offset], 0 /* dummy parameter */);
    }

    /**
     * Create a new, smaller, slice by limiting the size to 'Limit'.
     * 'Limit' must be smaller than or equal to 'Size'.
     * (this is checked at compile-time)
     */
    template<size_t Limit>
    slice<T, Limit> subslice() const
    {
        static_assert(Limit <= Size, "static slice size");
        return slice<T, Limit>(_data, 0 /* dummy parameter */);
    }

    /**
     * Create a new, smaller, slice by limiting the size to 'limit'.
     * 'limit' must be smaller than or equal to 'size()', or a zero length slice will be
     * returned.
     */
    slice<T, 0> subslice(size_t const limit) const
    {
        return slice<T, 0>(_data, (limit <= size()) ? limit : 0U);
    }

    /**
     * Create a new slice of a different data type.
     * The size of the new slice will be the maximum number
     * of elements that fit into the memory.
     * The new size must be at least 1, meaning the size of
     * the new type must be smaller than or equal to the
     * size of the memory referred to by this slice.
     * (This is checked at compile time)
     */
    template<class NewType>
    slice<NewType, (sizeof(T) * Size) / sizeof(NewType)> reinterpret_as() const
    {
        static_assert(sizeof(NewType) <= (sizeof(T) * Size), "static slice size");
        return slice<NewType, (sizeof(T) * Size) / sizeof(NewType)>(
            reinterpret_cast<NewType*>(_data), 0 /* dummy parameter */
        );
    }

    /**
     * Returns iterator to beginning of this slice.
     */
    iterator begin() const { return _data; }

    /**
     * Returns iterator to end of this slice.
     */
    iterator end() const { return &_data[Size]; }

    /**
     * Returns a reverse iterator pointing to the last element in the slice
     * (i.e., it's reverse beginning). Reverse iterators iterate backwards.
     *
     * \return A reverse iterator the reverse beginning of the slice (end).
     */
    reverse_iterator rbegin() { return reverse_iterator(end()); }

    /**
     * Returns a reverse iterator pointing to the first element in the slice
     * (i.e., it's reverse end). Reverse iterators iterate backwards.
     *
     * \return A reverse iterator to the reverse end (beginning).
     */
    reverse_iterator rend() { return reverse_iterator(begin()); }

    /**
     * Returns a const reverse iterator pointing to the last element in the slice
     * (i.e., it's reverse beginning). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator the reverse beginning of the slice (end).
     */
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

    /**
     * Returns a const reverse iterator pointing to the first element in the slice
     * (i.e., it's reverse end). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator to the reverse end (beginning).
     */
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    /**
     * Returns const_iterator to beginning of this slice.
     */
    const_iterator cbegin() const { return begin(); }

    /**
     * Returns const_iterator to end of this slice.
     */
    const_iterator cend() const { return end(); }

    /**
     * Returns a const reverse iterator pointing to the last element in the slice
     * (i.e., it's reverse beginning). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator the reverse beginning of the slice (end).
     */
    const_reverse_iterator crbegin() const { return rbegin(); }

    /**
     * Returns a const reverse iterator pointing to the first element in the slice
     * (i.e., it's reverse end). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator to the reverse end (beginning).
     */
    const_reverse_iterator crend() const { return rend(); }

    // [PUBLIC_API_END]
};

/**
 * A wrapper around a pointer providing save access to a region
 * of memory interpreted as a particular data type 'T'.
 * This is a specialization of the slice<T,Size> template for the case Size == 0.
 * Size being zero results in the size being managed at runtime.
 * access at runtime.
 */
template<class T>
class slice<T, 0>
{
public:
    /** Template parameter T */
    using value_type             = T;
    /** Unsigned integral type */
    using size_type              = std::size_t;
    /** Signed integral type */
    using difference_type        = std::ptrdiff_t;
    /** Reference to T */
    using reference              = T&;
    /** Const reference to T */
    using const_reference        = T const&;
    /** Pointer to T */
    using pointer                = T*;
    /** Pointer to const T */
    using const_pointer          = T const*;
    /** Random access iterator to value_type */
    using iterator               = pointer;
    /** Random access iterator to const value_type */
    using const_iterator         = const_pointer;
    /** Reverse iterator */
    using reverse_iterator       = std::reverse_iterator<iterator>;
    /** Constant reverse iterator */
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    size_t _size = 0U;
    T* _data     = nullptr;

    constexpr slice(T* const theData, size_t const theSize) : _size(theSize), _data(theData) {}

    template<class OtherT, size_t OtherSize>
    friend class slice;

public:
    /**
     * The default constructor creates a zero length slice.
     */
    constexpr slice() = default;

    /**
     * Creation from anything having .data() and .size() methods (like estd::vector<T>).
     */
    template<class Data>
    constexpr slice(
        Data& theData,
        typename std::enable_if<
            internal::_has_size<Data>::value
            && (!std::is_same<Data, slice>::value)>::type* = nullptr)
    : _size(theData.size()), _data(theData.data())
    {}

    /**
     * Creation from anything having .data() and .size() methods (like estd::vector<T>).
     */
    template<class Data>
    constexpr slice(
        Data&& theData,
        typename std::enable_if<
            internal::_has_size<Data>::value
            && (!std::is_same<Data, slice>::value)>::type* = nullptr)
    : _size(theData.size()), _data(theData.data())
    {}

    /**
     * Creation from anything having .data() and .size() methods (like estd::vector<T>).
     */
    template<class Data>
    constexpr slice(
        Data const& theData,
        typename ::std::enable_if<
            internal::_has_size<Data>::value
            && (!std::is_same<Data, slice>::value)>::type* = nullptr)
    : _size(theData.size()), _data(theData.data())
    {}

    /**
     * Creation from a C-Array.
     */
    template<size_t OtherSize>
    constexpr slice(T (&theData)[OtherSize]) : _size(OtherSize), _data(theData)
    {}

    /**
     * Construct a slice referring to the memory given by a pointer.
     * Since this operation is not able to do any range checking,
     * the caller must ensure 'data' points to a valid memory region
     * of appropriate size.
     */
    static slice from_pointer(T* const theData, size_t const theSize)
    {
        return slice(theData, theSize);
    }

    /**
     * Return the size of the slice.
     */
    size_t size() const { return _size; }

    /**
     * Return the raw pointer to the data.
     */
    T* data() const { return _data; }

    /**
     * Return a reference to the element of the slice at the
     * specified location.
     * This operator will not do any bounds checking.
     */
    T& operator[](size_t const idx) const { return _data[idx]; }

    /**
     * Return a reference to the element of the slice at the
     * specified location.
     * This method will assert if the index is out of bounds.
     */
    T& at(size_t const idx) const
    {
        estd_assert(idx < size());
        return _data[idx];
    }

    /**
     * Create a new, smaller, slice by skipping 'Offset' elements
     * from the beginning of this slice.
     * 'offset' must be smaller than 'size()', or a zero length slice will be returned.
     */
    slice offset(size_t const position) const
    {
        slice<T, 0> s(*this);
        s.advance(position);
        return s;
    }

    /**
     * Create a new, smaller, slice by limiting the size to 'limit'.
     * 'limit' must be smaller than or equal to 'size()', or a zero length slice will be
     * returned.
     */
    slice subslice(size_t const limit) const { return slice(*this).trim(limit); }

    /**
     * Advances this slice by 'offset' elements and return a reference to this (modified) slice.
     * 'offset' must be smaller than size() or a zero length slice will result.
     */
    slice& advance(size_t offset)
    {
        offset = (offset >= _size) ? _size : offset;
        _size -= offset;
        _data += offset;
        return *this;
    }

    /**
     * Make slice smaller, by limiting the size to 'limit' elements.
     * 'limit' must be smaller than or equal to 'size()', or the resulting size will be zero.
     */
    slice& trim(size_t const limit)
    {
        _size = (limit <= size()) ? limit : 0U;
        return *this;
    }

    /**
     * Create a new slice of a different data type.
     * The size of the new slice will be the maximum number
     * of elements that fit into the memory.
     */
    template<class NewType>
    slice<NewType, 0> reinterpret_as() const
    {
        return slice<NewType, 0>(
            reinterpret_cast<NewType*>(_data), (sizeof(T) * size()) / sizeof(NewType));
    }

    /**
     * Returns iterator to beginning of this slice.
     */
    iterator begin() const { return _data; }

    /**
     * Returns iterator to end of this slice.
     */
    iterator end() const { return &_data[_size]; }

    /**
     * Returns a reverse iterator pointing to the last element in the slice
     * (i.e., it's reverse beginning). Reverse iterators iterate backwards.
     *
     * \return A reverse iterator the reverse beginning of the slice (end).
     */
    reverse_iterator rbegin() { return reverse_iterator(end()); }

    /**
     * Returns a reverse iterator pointing to the first element in the slice
     * (i.e., it's reverse end). Reverse iterators iterate backwards.
     *
     * \return A reverse iterator to the reverse end (beginning).
     */
    reverse_iterator rend() { return reverse_iterator(begin()); }

    /**
     * Returns a const reverse iterator pointing to the last element in the slice
     * (i.e., it's reverse beginning). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator the reverse beginning of the slice (end).
     */
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

    /**
     * Returns a const reverse iterator pointing to the first element in the slice
     * (i.e., it's reverse end). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator to the reverse end (beginning).
     */
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    /**
     * Returns const_iterator to beginning of this slice.
     */
    const_iterator cbegin() const { return begin(); }

    /**
     * Returns const_iterator to end of this slice.
     */
    const_iterator cend() const { return end(); }

    /**
     * Returns a const reverse iterator pointing to the last element in the slice
     * (i.e., it's reverse beginning). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator the reverse beginning of the slice (end).
     */
    const_reverse_iterator crbegin() const { return rbegin(); }

    /**
     * Returns a const reverse iterator pointing to the first element in the slice
     * (i.e., it's reverse end). Reverse iterators iterate backwards.
     *
     * \return A const reverse iterator to the reverse end (beginning).
     */
    const_reverse_iterator crend() const { return rend(); }
};

template<>
class slice<void, 0>
{
public:
    size_t size() const { return 0U; }

    void* data() const { return nullptr; }
};

template<class NewType, class T, size_t Size>
slice<NewType, (sizeof(T) * Size) / sizeof(NewType)> reinterpret_as(slice<T, Size> const o)
{
    return o.template reinterpret_as<NewType>();
}

// [PUBLIC_NON_MEMBER_API_BEGIN]
inline slice<uint8_t const> make_str(char const* const s)
{
    size_t i = 0;
    while (s[i] != 0U)
    {
        ++i;
    }
    return slice<uint8_t const>::from_pointer(reinterpret_cast<uint8_t const*>(s), i);
}

/**
 * Create a slice from a C array without needing
 * to explicitly specify the type or length.
 * The resulting slice will be runtime sized.
 */
template<class T, size_t OtherSize>
constexpr slice<T, 0> make_slice(T (&data)[OtherSize])
{
    return slice<T, OtherSize>(data);
}

/**
 * Create a slice from any contiguous container that provides
 * a .data() and .size() method without needing to explicitly specify the type.
 * The resulting slice will be runtime sized.
 */
template<class T>
constexpr slice<typename T::value_type> make_slice(T& data)
{
    return slice<typename T::value_type>(data);
}

/**
 * Create a slice from a C array without needing
 * to explicitly specify the type or length.
 * The resulting slice will be statically sized.
 */
template<class T, size_t OtherSize>
constexpr slice<T, OtherSize> make_static_slice(T (&data)[OtherSize])
{
    return slice<T, OtherSize>(data);
}

/**
 * Create a slice from an estd::array without needing
 * to explicitly specify the type or length.
 * The resulting slice will be statically sized.
 */
template<class T, size_t OtherSize>
constexpr slice<T, OtherSize> make_static_slice(array<T, OtherSize>& data)
{
    return slice<T, OtherSize>(data);
}

/**
 * Create a slice from a const estd::array without needing
 * to explicitly specify the type or length.
 * The resulting slice will be statically sized.
 */
template<class T, size_t OtherSize>
constexpr slice<T const, OtherSize> make_static_slice(array<T, OtherSize> const& data)
{
    return slice<T const, OtherSize>(data);
}

// [PUBLIC_NON_MEMBER_API_END]

// clang-format off
static_assert(std::is_trivially_copyable<slice<uint8_t>>::value, "slice is trivially copyable");
static_assert(std::is_trivially_copyable<slice<uint8_t const>>::value, "slice of const is trivially copyable");
static_assert(std::is_trivially_destructible<slice<uint8_t>>::value, "slice is trivially destructible");
static_assert(std::is_trivially_destructible<slice<uint8_t const >>::value, "slice of const is trivially destructible");
// clang-format on

} // namespace estd

#define BytesAreSlice(S) ElementsAreArray(S.data(), S.size())
#endif // GUARD_2626BDF2_63F2_448A_B810_5929FA66F5FF
