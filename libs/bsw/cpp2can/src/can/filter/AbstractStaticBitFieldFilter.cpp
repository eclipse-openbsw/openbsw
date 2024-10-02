// Copyright 2024 Accenture.

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
