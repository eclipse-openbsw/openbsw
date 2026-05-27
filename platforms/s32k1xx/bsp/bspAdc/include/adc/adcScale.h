/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

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
