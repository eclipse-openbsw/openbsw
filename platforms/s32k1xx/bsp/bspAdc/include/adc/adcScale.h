// Copyright 2024 Accenture.

#ifndef GUARD_DA333343_F48F_41FF_B051_1449F1C46AF6
#define GUARD_DA333343_F48F_41FF_B051_1449F1C46AF6

#include "platform/estdint.h"

namespace bios
{

template<typename InternCalculat, uint32_t A, uint32_t B, uint32_t C>
uint32_t scaleFunction(uint16_t y);

/**
 * Linear Scaling
 *   x= y*(A/B)+- C
 */

template<typename InternCalculat, uint32_t A, uint32_t B, uint32_t C>
uint32_t scaleFunction(uint16_t y)
{
    InternCalculat calcValue = 0;
    calcValue                = A * y;
    calcValue                = calcValue / B;
    calcValue += C;

    return (uint32_t)(calcValue);
}

} // namespace bios
#endif /* GUARD_DA333343_F48F_41FF_B051_1449F1C46AF6 */
