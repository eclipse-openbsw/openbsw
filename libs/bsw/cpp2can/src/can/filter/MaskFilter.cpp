/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "can/filter/MaskFilter.h"

#include <etl/utility.h>

namespace can
{
MaskFilter::MaskFilter() : _mask(0U), _pattern(0U), _empty(true) {}

MaskFilter::MaskFilter(uint32_t const from, uint32_t const to) : MaskFilter() { add(from, to); }

// virtual
void MaskFilter::add(uint32_t const filterId) { widen(0xFFFFFFFFU, filterId); }

// virtual
void MaskFilter::add(uint32_t from, uint32_t to)
{
    if (from > to)
    {
        ::ETL_OR_STD::swap(from, to);
    }
    // find the number of low order bits that need to be unconstrained ("don't care") to cover
    // the whole (inclusive) range [from, to] with a single mask/pattern block.
    uint32_t blockMask  = 0xFFFFFFFFU;
    uint32_t difference = from ^ to;
    while (difference != 0U)
    {
        blockMask <<= 1U;
        difference >>= 1U;
    }
    widen(blockMask, from & blockMask);
}

// virtual
bool MaskFilter::match(uint32_t const filterId) const
{
    return (!_empty) && ((filterId & _mask) == _pattern);
}

void MaskFilter::clear()
{
    _mask    = 0U;
    _pattern = 0U;
    _empty   = true;
}

void MaskFilter::open()
{
    _mask    = 0U;
    _pattern = 0U;
    _empty   = false;
}

void MaskFilter::widen(uint32_t const mask, uint32_t const pattern)
{
    if (_empty)
    {
        _mask    = mask;
        _pattern = pattern & mask;
        _empty   = false;
    }
    else
    {
        // keep only the bits that are fixed in both the current and the new mask/pattern and
        // that agree on the fixed value; this yields the smallest mask/pattern block covering
        // both.
        uint32_t const commonMask = _mask & mask & ~(_pattern ^ pattern);
        _mask                     = commonMask;
        _pattern &= commonMask;
    }
}

} // namespace can
