// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

#include <utility>

namespace estd
{
/**
 * Provides transparent wrapper for global objects that
 * are never destructed. The type will become trivially
 * destructible.
 *
 * @tparam T type to be instantiated.
 */
template<typename T>
class alignas(T) indestructible final
{
    // Array is not initialized as this seems
    // to produce a lot of side effects with Diab
    uint8_t _data[sizeof(T)];

    indestructible(indestructible const&)            = delete;
    indestructible& operator=(indestructible const&) = delete;

public:
    template<typename... Args>
    ESR_ALWAYS_INLINE indestructible(Args&&... args)
    {
        new (_data) T(std::forward<Args>(args)...);
    }

    ~indestructible() = default;

    ESR_ALWAYS_INLINE
    T& get() { return *reinterpret_cast<T*>(_data); }

    ESR_ALWAYS_INLINE
    T* operator->() { return reinterpret_cast<T*>(_data); }

    ESR_ALWAYS_INLINE
    operator T&() { return get(); }
};

} // namespace estd

