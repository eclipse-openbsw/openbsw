// Copyright 2024 Accenture.

#include "can/filter/IntervalFilter.h"

#include <etl/algorithm.h>

#include <algorithm>

namespace can
{
IntervalFilter::IntervalFilter() : _from(MAX_ID), _to(0U) {}

IntervalFilter::IntervalFilter(uint32_t const from, uint32_t const to) : IntervalFilter()
{
    IntervalFilter::add(from, to);
}

// virtual
void IntervalFilter::add(uint32_t const filterId) { add(filterId, filterId); }

void IntervalFilter::add(uint32_t from, uint32_t to)
{
    if (from > MAX_ID)
    {
        from = MAX_ID;
    }
    if (to > MAX_ID)
    {
        to = MAX_ID;
    }
    // assert order
    if (from > to)
    {
        ::std::swap(from, to);
    }
    // adjust lower bound
    _from = ::etl::min(_from, from);
    // adjust upper bound
    _to   = ::etl::max(_to, to);
}

// virtual
bool IntervalFilter::match(uint32_t const filterId) const
{
    return (filterId >= _from) && (filterId <= _to);
}

void IntervalFilter::clear()
{
    _from = MAX_ID;
    _to   = 0U;
}

void IntervalFilter::open()
{
    _from = 0x00U;
    _to   = MAX_ID;
}

} /*namespace can*/
