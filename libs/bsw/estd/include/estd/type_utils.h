// Copyright 2024 Accenture.

#ifndef GUARD_33A284CC_0E02_469B_8C57_1757F4CC8AC0
#define GUARD_33A284CC_0E02_469B_8C57_1757F4CC8AC0

#include <platform/estdint.h>

#include <type_traits>

namespace estd
{
/**
 * A utility class to encapsulate the reinterpret_cast to a single
 * location.
 */
template<class T>
class type_utils
{
public:
    /**
     * Casts the underlying raw data to an object of type T *.
     *
     * \param data A pointer to the raw data of an array of objects of type T.
     * \param index The offset in T's to the requested object.
     */
    static T* cast_to_type(uint8_t* const data, std::size_t const idx = 0U)
    {
        return reinterpret_cast<T*>(data + (idx * sizeof(T)));
    }

    /**
     * Calls the destructor for an object of type T by
     * casting the data pointer to a T *. Then calling the
     * T destructor.
     *
     * \param data A pointer to the raw data of an array of objects of type T.
     * \param index The offset in T's to the requested object.
     */
    static void destroy(uint8_t* const data, std::size_t const idx = 0U)
    {
        cast_to_type(data, idx)->~T();
    }

    /**
     * Casts the data pointer to a raw uint8_t pointer.
     *
     * \param data The data object of type T
     * \return A pointer to the start of the data as a uint8_t pointer.
     */
    static uint8_t* cast_to_raw(T* const data) { return reinterpret_cast<uint8_t*>(data); }

    /**
     * Casts the underlying raw data to an object of type const T *.
     *
     * \param data A pointer to the raw data of an array of objects of type T.
     * \param index The offset in T's to the requested object.
     */
    static T const* cast_const_to_type(uint8_t const* const data, std::size_t const idx = 0U)
    {
        return reinterpret_cast<T const*>(data + (idx * sizeof(T)));
    }

    static T* cast_from_void(void* const obj) { return static_cast<T*>(obj); }

    static T const* const_cast_from_void(void const* const obj)
    {
        return static_cast<T const*>(obj);
    }

    static void* cast_to_void(T* const obj) { return static_cast<void*>(obj); }

    static void* const_cast_to_void(T const* const obj)
    {
        return const_cast<void*>(static_cast<void const*>(obj));
    }
};

/**
 * Convert a pointer to an integer of same size
 * \param p pointer to convert
 * \return integer value expressing the pointer value
 */
template<class P>
uintptr_t integer_from_pointer(P const p)
{
    static_assert(std::is_pointer<P>::value, "");
    return reinterpret_cast<uintptr_t>(p);
}

/**
 * Convert a integer with same size as a pointer to pointer type.
 * \param address integer value of address to convert
 * \return pointer to desired time with the given address
 */
template<class P>
P pointer_from_integer(uintptr_t const address)
{
    static_assert(std::is_pointer<P>::value, "");
    return (P)address;
}

/**
 * Helper type for the underlying type of enumerations.
 * Based on std::underlying_type_t from C++14
 */
template<class Enum>
using underlying_type_t = typename ::std::underlying_type<Enum>::type;

/**
 * Converts an enumeration to its underlying type.
 * Based on std::to_underlying from C++23
 * \param e enumeration value to convert
 * \return integer value of the underlying type of Enum
 */
template<class Enum>
constexpr underlying_type_t<Enum> to_underlying(Enum const e) noexcept
{
    return static_cast<underlying_type_t<Enum>>(e);
}

} // namespace estd

#endif /* GUARD_33A284CC_0E02_469B_8C57_1757F4CC8AC0 */
