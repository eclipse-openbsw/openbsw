/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "can/filter/AbstractStaticBitFieldFilter.h"

namespace can
{
// virtual
bool AbstractStaticBitFieldFilter::match(uint32_t const filterId) const
{
    if (filterId <= MAX_ID)
    {
        uint16_t const idx = static_cast<uint16_t>(filterId / 8U);

        uint16_t const maskIndex = static_cast<uint16_t>(1U) << (filterId % 8U);
        return (static_cast<uint16_t>(getMaskValue(idx)) & maskIndex) == maskIndex;
    }
    return false;
}

} // namespace can
