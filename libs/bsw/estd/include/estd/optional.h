// Copyright 2024 Accenture.

/**
 * Contains estd::optional
 * \file
 * \ingroup estl_containers
 */
#pragma once

#include "estd/assert.h"
#include "estd/constructor.h"
#include "estd/none.h"
#include "estd/type_traits.h"

#include <cstring>
#include <type_traits>

namespace estd
{
/**
 * A wrapper representing a value that may or not be valid.
 * The instance of type T is stored in memory residing inside
 * the optional<T> object itself.
 */
template<typename T>
class optional
{
public:
    using value_type = T;

    optional() = default;

    /**
     * The copy constructor copies both the contained object (if there is one), as well
     * as the validity state.
     * Note that this uses the copy constructor of T and thus only works for copyable types.
     */
    optional(optional const& other) { *this = other; }

    optional(T const& other) { emplace().construct(other); }

    template<typename U>
    optional(
        U const& value,
        typename std::enable_if<
            std::is_same<T, U>::value && (!std::is_trivially_copyable<T>::value)>::type const* const
        = nullptr)
    {
        (void)emplace().construct(value);
    }

    template<typename U>
    optional(
        U const& value,
        typename std::enable_if<
            std::is_same<T, U>::value && std::is_trivially_copyable<T>::value>::type const* const
        = nullptr)
    {
        _has_value = true;
        (void)::memcpy(_value._data, &value, sizeof(value));
    }

    optional& operator=(none_t const)
    {
        reset();
        return *this;
    }

    optional& operator=(T const& other)
    {
        operator=(optional(other));
        return *this;
    }

    optional& operator=(optional const& other)
    {
        if (&other != this)
        {
            this->reset();
            if (other.has_value())
            {
                (void)this->emplace().construct(*other);
            }
        }
        return *this;
    }

    explicit operator bool() const { return _has_value; }

    bool has_value() const { return _has_value; }

    T& get()
    {
        estd_assert(_has_value);
        return *_value.cast_to_type();
    }

    T const& get() const
    {
        estd_assert(_has_value);
        return *_value.cast_const_to_type();
    }

    T& operator*() { return get(); }

    T const& operator*() const { return get(); }

    T const* operator->() const { return &get(); }

    T* operator->() { return &get(); }

    /**
     * Construct a new instance of T inside the memory contained in the optional.
     * This will set the optional to valid.
     */
    constructor<T> emplace()
    {
        reset();
        _has_value = true;
        return constructor<T>(_value.data);
    }

    /**
     * Empty the optional.
     * This will call the destructor of T.
     */
    void reset()
    {
        destroy<T>();
        _has_value = false;
    }

    /**
     * Shortcut to default to a fallback value.
     * The return value will either be a copy of the contained object (if there is one),
     * or a copy the provided fallback parameter.
     * Note that the result is returned as a copy. This in unavoidable because most of the
     * time a temporary object will be passed as fallback parameter a reference to it would
     * be dangling after the function returns. Therefore use this method should only be used
     * for simple types.
     */
    T value_or(T const& fallback) const { return has_value() ? get() : fallback; }

    ~optional() { destroy<T>(); }

private:
    aligned_mem<T> _value;
    bool _has_value = false;

    /**
     * Call the destructor of T
     */
    template<typename U>
    inline void destroy(
        typename std::enable_if<(std::is_trivially_destructible<U>::value == 0)>::type const* const
        = nullptr)
    {
        if (this->has_value())
        {
            // 3803,3804: FP: Destructors do not have return values
            // 4208:      item is used because we call the destructor on the object
            T& item = get();
            item.~T();
        }
    }

    /**
     * Overload for trivially destructible types
     */
    template<typename U>
    inline void destroy(
        typename std::enable_if<std::is_trivially_destructible<U>::value>::type const* const
        = nullptr)
    {}
};

/**
 * An optional reference. In contrast to the non-reference optional this does not contain
 * any memory to store an instance of T.
 * Instead optional<T&> is meant as a safer, drop-in replacement for raw-pointers.
 * It will assert when trying to dereference a null pointer instead of leading to undefined
 * behaviour.
 */
template<typename T>
class optional<T&>
{
public:
    using value_type = T;

    optional() : _value(nullptr) {}

    optional(T& target) { reset(target); }

    bool has_value() const { return nullptr != _value; }

    T& get() const
    {
        estd_assert(has_value());
        return *_value;
    }

    T& operator*() const { return get(); }

    T* operator->() const { return &get(); }

    /**
     * Make the optional reference point to nothing.
     */
    void reset() { _value = nullptr; }

    /**
     * Make the optional reference point to a new target.
     */
    void reset(T& target) { _value = &target; }

private:
    T* _value;
};

template<class T>
inline bool operator==(optional<T> const& lhs, optional<T> const& rhs)
{
    bool const lhs_valid = static_cast<bool>(lhs.has_value());
    bool const rhs_valid = static_cast<bool>(rhs.has_value());
    if (lhs_valid && rhs_valid)
    {
        return (*lhs == *rhs);
    }
    return (lhs_valid == rhs_valid);
}

template<class T>
inline bool operator!=(optional<T> const& lhs, optional<T> const& rhs)
{
    return !(lhs == rhs);
}
} // namespace estd
