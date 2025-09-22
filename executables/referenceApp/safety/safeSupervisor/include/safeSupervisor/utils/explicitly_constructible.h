// Copyright 2025 Accenture.

#pragma once

#include <platform/estdint.h>

namespace safety
{
/**
 * This tag is being used to make the compiler use the very specific new operator defined below.
 * Otherwise the compiler will complain about redefinition where a <new> was included (e.g. in the
 * unit test)
 */
struct new_operator_tag;

/**
 * Container which makes it possible to defer an objects construction. The intention behind it is to
 * explicitly construct objects in the safety context _after_ the MPU was activated.
 * \param T The type of object to wrap
 * \note The correct alignment of T is ensured
 */
template<typename T>
class explicitly_constructible
{
public:
    /**
     * Explicitly constructs the object.
     * \tparam Ts C'tor parameter types
     * \param params C'tor parameter values
     */
    template<typename... Ts>
    void construct(Ts&&... params)
    {
        new (reinterpret_cast<new_operator_tag*>(&_mem[0])) T(static_cast<Ts&&>(params)...);
    }

    /**
     * Gives access to the object.
     * \note For performance reasons, there is no check if the object was constructed before.
     */
    T& operator*() { return *reinterpret_cast<T*>(&_mem[0]); }

private:
    alignas(alignof(T)) uint8_t _mem[sizeof(T)];
    T const* const _obj{reinterpret_cast<T*>(&_mem[0])}; // only for better debugging
};
} // namespace safety

/**
 * Default implementation for placement new operator.
 */
inline void* operator new(size_t const /* count */, ::safety::new_operator_tag* const ptr)
{
    return ptr;
}
