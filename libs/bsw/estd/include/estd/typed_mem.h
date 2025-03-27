// Copyright 2024 Accenture.

#pragma once

#include "estd/assert.h"
#include "estd/type_traits.h"

#include <platform/estdint.h>

#include <new>
#include <utility>

namespace estd
{

/**
 * Wrapper class that provides a memory area and lets the user emplace and instance of
 * T in this memory at runtime. This class also erases the destructor call of T, i.e.
 * if typed_mem goes out of scope, the destructor if the wrapped type will not be called.
 * This can be done explicitly by calling destroy().
 * [TPARAMS_BEGIN]
 * \tparam T    Type of element stored in this instance of typed_mem.
 * [TPARAMS_END]
 */
template<class T>
class typed_mem final
{
public:
    // [PUBLIC_TYPES_BEGIN]
    /** Type of value stored in this typed_mem. */
    using value_type      = T;
    /** A reference to the template parameter T */
    using reference       = T&;
    /** A const reference to the template parameter T */
    using const_reference = T const&;
    /** A pointer to the template parameter T */
    using pointer         = T*;
    /** A const pointer to the template parameter T */
    using const_pointer   = T const*;

    /** Tag type to construct an instance of typed_mem which zeros out the internal byte array. */
    struct zero_data
    {};

    // [PUBLIC_TYPES_END]

    // [PUBLIC_API_BEGIN]
    /**
     * Constructs an instance of typed_mem without clearing the internal byte array.
     */
    typed_mem();

    /**
     * Constructs an instance of typed_mem zeroing out the internal byte array.
     */
    explicit typed_mem(zero_data);

    /**
     * Default destructor which will NOT call the destructor of the object which was created
     * by calling emplace().
     */
    ~typed_mem() = default;

    /**
     * Calls the destructor of the wrapped object and asserts if has_value() is false.
     */
    void destroy();

    /**
     * \returns true if object has been constructed using emplace().
     * \returns false otherwise.
     */
    bool has_value() const;

    /**
     * Constructs the instance of T forwarding the given \p args to its constructor and
     * asserts if has_value() is true before calling emplace().
     *
     * \returns the instance of T which has been constructed in the internal byte array.
     */
    template<typename... Args>
    reference emplace(Args&&... args);

    /**
     * \returns a pointer of type T and asserts if has_value() is false.
     */
    pointer operator->();

    /**
     * \returns a const pointer of type T and asserts if has_value() is false.
     */
    const_pointer operator->() const;

    /**
     * \returns reference of type T and asserts if has_value() is false.
     */
    reference operator*();

    /**
     * \returns const reference of type T and asserts if has_value() is false.
     */
    const_reference operator*() const;

    // [PUBLIC_API_END]
private:
    // Internal buffer to hold an instance of value_type.
    aligned_mem<value_type> _data;
    // Pointer to value_type for debugging of _data and implementation of has_value().
    value_type* _value;
};

template<class T>
inline typed_mem<T>::typed_mem() : _value(nullptr)
{}

template<class T>
inline typed_mem<T>::typed_mem(zero_data) : _data(), _value(nullptr)
{}

template<class T>
inline void typed_mem<T>::destroy()
{
    estd_assert(has_value());
    _value->~T();
    _value = nullptr;
}

template<class T>
inline bool typed_mem<T>::has_value() const
{
    return _value != nullptr;
}

template<class T>
template<typename... Args>
inline auto typed_mem<T>::emplace(Args&&... args) -> value_type&
{
    estd_assert(!has_value());
    _value = _data.cast_to_type();
    return *new (_value) value_type(std::forward<Args>(args)...);
}

template<class T>
inline auto typed_mem<T>::operator->() -> pointer
{
    estd_assert(has_value());
    return _data.cast_to_type();
}

template<class T>
inline auto typed_mem<T>::operator->() const -> const_pointer
{
    return operator->();
}

template<class T>
inline auto typed_mem<T>::operator*() -> reference
{
    return *operator->();
}

template<class T>
inline auto typed_mem<T>::operator*() const -> const_reference
{
    return *this;
}

} // namespace estd
