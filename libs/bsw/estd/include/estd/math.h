// Copyright 2024 Accenture.

#ifndef GUARD_10482EE4_8A08_43A4_9717_09276F7804B8
#define GUARD_10482EE4_8A08_43A4_9717_09276F7804B8
#include "estd/result.h"

#include <cstdint>
#include <type_traits>

namespace estd
{

template<typename ReturnType = uint64_t, typename InputType>
ReturnType abs(InputType const& input)
{
    static_assert(
        ((std::is_integral<ReturnType>::value && std::is_integral<InputType>::value)
         || (std::is_floating_point<ReturnType>::value
             && std::is_floating_point<InputType>::value)),
        "The ReturnType and InputType have to be both integral or both floating point types.");
    static_assert(
        (((((std::is_unsigned<ReturnType>::value || std::is_floating_point<ReturnType>::value))
           && (sizeof(ReturnType) >= sizeof(InputType)))
          || (sizeof(ReturnType) > sizeof(InputType)))),
        "If the ReturnType is unsigned or a floating point, it needs to be at least the same size "
        "as the InputType, or else; the ReturnTypes size needs to be strictly bigger than the "
        "InputType.");
    if (input >= static_cast<InputType>(0))
    {
        return static_cast<ReturnType>(input);
    }
    return -static_cast<ReturnType>(input);
}

} // namespace estd

#endif /* GUARD_10482EE4_8A08_43A4_9717_09276F7804B8 */
