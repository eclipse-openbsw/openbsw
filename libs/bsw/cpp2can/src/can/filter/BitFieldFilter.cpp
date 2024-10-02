// Copyright 2024 Accenture.

#include "can/filter/BitFieldFilter.h"

namespace can
{
#ifdef __GNUC__
uint16_t const BitFieldFilter::NUMBER_OF_BITS;
uint16_t const BitFieldFilter::MAX_ID;
uint16_t const BitFieldFilter::MASK_SIZE;
#endif

BitFieldFilter::BitFieldFilter() { BitFieldFilter::clear(); }

// virtual
void BitFieldFilter::add(uint32_t const filterId)
{
    if (filterId <= MAX_ID)
    {
        uint16_t const idx = static_cast<uint16_t>(filterId / 8U);

        _mask[idx]
            = static_cast<uint8_t>((_mask[idx] | static_cast<uint8_t>(1U << (filterId % 8U))));
    }
}

// virtual
void BitFieldFilter::add(uint32_t const from, uint32_t const to)
{
    for (uint32_t i = from; i <= to; ++i)
    {
        add(i);
    }
}

// virtual
bool BitFieldFilter::match(uint32_t const filterId) const
{
    if (filterId <= MAX_ID)
    {
        uint16_t const idx = static_cast<uint16_t>(filterId / 8U);

        uint16_t const maskIndex = static_cast<uint16_t>(1U) << (filterId % 8U);
        return (static_cast<uint16_t>(_mask[idx]) & maskIndex) == maskIndex;
    }
    return false;
}

} // namespace can
