/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "shed/multi_list.h"

#include <cstdint>

namespace shed
{
namespace internal
{
size_t multi_list::move_node(size_t const s, size_t dst)
{
    while ((s < items()[dst]) && (items()[dst] < _n))
    {
        dst = items()[dst];
    }
    auto const src            = backlinks()[s];
    items()[src]              = items()[s];
    items()[s]                = items()[dst];
    items()[dst]              = static_cast<idx_type>(s);
    backlinks()[items()[src]] = src;
    backlinks()[items()[s]]   = static_cast<idx_type>(s);
    backlinks()[items()[dst]] = static_cast<idx_type>(dst);
    return dst;
}

multi_list* multi_list::make(size_t const n, size_t const buckets, ::etl::span<uint8_t>& mem)
{
    if (mem.size() < memory_for(n, buckets))
    {
        return nullptr;
    }
    // If the provided memory isn't aligned to the requirements of idx_type, don't continue
    if ((reinterpret_cast<uintptr_t>(mem.data()) % static_cast<uintptr_t>(sizeof(idx_type))) != 0)
    {
        return nullptr;
    }
    auto const self = new (mem.data()) multi_list(n, buckets);
    mem.advance(memory_for(n, buckets));
    return self;
}

multi_list::multi_list(size_t const n, size_t const buckets)
{
    _n         = static_cast<idx_type>(n);
    _buckets   = static_cast<idx_type>(buckets);
    items()[0] = static_cast<idx_type>(n);
    for (size_t i = 0; i < n; ++i)
    {
        items()[i + 1] = static_cast<idx_type>(i);
    }
    for (size_t i = n + 1; i < n + buckets; ++i)
    {
        items()[i] = static_cast<idx_type>(i);
    }
    for (size_t i = 0; i < n + buckets; ++i)
    {
        backlinks()[items()[i]] = static_cast<idx_type>(i);
    }
}

void multi_list::NotInBuckets::iter(::etl::delegate<bool(size_t)> const f) const
{
    size_t const n              = self->_n;
    idx_type const* const items = self->items();
    size_t pos[2]               = {items[n + buckets[0]], items[n + buckets[1]]};
    size_t i                    = 0;

    do
    {
        ++i;
        while (((n - i) == pos[0]) || ((n - i) == pos[1]))
        {
            ++i;
            auto const j = ((pos[0] < pos[1]) && (pos[1] < n)) ? 1U : 0U;
            pos[j]       = items[pos[j]];
        }
    } while ((i < n + 1) && f(n - i));
}
} // namespace internal
} // namespace shed
