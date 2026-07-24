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

#include "shed/move_op.h"

#include <etl/delegate.h>
#include <etl/span.h>

#include <cstddef>
#include <cstdint>
#include <limits>

// multi_list is a data structure to maintain membership information for N elements in M sets.
// The items are always the numbers 0..N-1, indended to be used as indicies into anther data
// structure on the side (in case of shed, multiple other arrays)
//
// Invariants:
//  - The total number of items and also sets/buckets stays constant
//  - Every index is a member of exactly one set at all times
//
// Characteristics:
//  - Memory consumption is O(N + M)
//  - Iteration of members of any set is O(Nx), with Nx being the number of elements in the given
//    set
//  - Iteration of members not in a set is O(N)
//  - Transfering of any number of elements from set A to set B is O(Na)
//  - Operations on empty sets are always O(1)
//  - All operations use seqential memory access patterns with high locality
//  - All operations are done in place and don't require extra memory
//
// Implementation:
//  The idea is to use M linked lists interleaved inside one shared array.
//  By using idicies instead of pointers a smaller type (uint16_t) than pointer size can be used
//  to save memory and increase locality.
//  To make set transfers faster, the lists are doubly-linked, with the backwards links stored
//  in a seperate array on the side to make forward iteration faster by not polluting the cpu cache
//  with backward links not needed for forward iteration.
//  The index of any element serves as it's address, but also as it's associated value/"payload"
//  Therefore no extra memory for "payload" is required.

namespace shed
{
namespace internal
{
struct multi_list
{
    using idx_type = uint16_t;

    static constexpr bool is_size_valid(size_t const n, size_t const buckets)
    {
        return (n + buckets) <= std::numeric_limits<idx_type>::max();
    }

    static constexpr size_t memory_for(size_t const n, size_t const buckets)
    {
        // (n + buckets) has to be <= maxint(idx_type), in order for multi_list to work;
        // because of a limited support for constexpr in diab we can't have an extra condition
        // in this function
        return sizeof(multi_list) + (2 * sizeof(idx_type) * (n + buckets));
    }

    struct InBucket
    {
        using value_type = size_t;
        idx_type const* items;
        size_t bucket_n;

        InBucket(multi_list const* const self_, size_t const bucket_)
        : items(self_->items()), bucket_n(self_->_n + bucket_)
        {}

        template<typename F>
        void iter(F const f) const
        {
            for (size_t pos = items[bucket_n]; pos != bucket_n; pos = items[pos])
            {
                if (!f(pos))
                {
                    break;
                }
            }
        }
    };

    struct NotInBucket
    {
        using value_type = size_t;
        multi_list const* const self;
        size_t bucket;

        NotInBucket(multi_list const* const self_, size_t const bucket_)
        : self(self_), bucket(bucket_)
        {}

        template<typename F>
        void iter(F&& f) const
        {
            size_t const n              = self->_n;
            idx_type const* const items = self->items();
            size_t pos                  = items[n + bucket];
            size_t i                    = 0;

            do
            {
                ++i;

                // This loop skips slots that are in the bucket
                while ((n - i) == pos)
                {
                    ++i;
                    pos = items[pos];
                }

            } while ((i < n + 1) && std::forward<F>(f)(n - i));
        }
    };

    struct NotInBuckets
    {
        using value_type = size_t;
        multi_list const* const self;
        size_t buckets[2];

        NotInBuckets(multi_list const* const self_, size_t const buckets_0, size_t const buckets_1)
        : self(self_), buckets{buckets_0, buckets_1}
        {}

        void iter(::etl::delegate<bool(size_t)> f) const;
    };

    InBucket in_bucket(size_t const bucket) const { return InBucket(this, bucket); }

    NotInBucket not_in_bucket(size_t const bucket) const { return NotInBucket(this, bucket); }

    NotInBuckets not_in_buckets(size_t const bucket0, size_t const bucket1) const
    {
        return NotInBuckets(this, bucket0, bucket1);
    }

    template<typename F>
    size_t move_if(size_t src, size_t dst, F&& f)
    {
        src          = src + _n;
        dst          = dst + _n;
        size_t moved = 0;

        while (items()[src] < _n)
        {
            auto const s = items()[src];
            auto const r = std::forward<F>(f)(s);
            if ((r == move_op::MOVE) || (r == move_op::MOVE_DONE))
            {
                dst = move_node(s, dst);
                ++moved;
            }
            else
            {
                src = items()[src];
            }
            if ((r == move_op::SKIP_DONE) || (r == move_op::MOVE_DONE))
            {
                break;
            }
        }
        return moved;
    }

    void move_idx(size_t const s, size_t const dst) { (void)move_node(s, dst + _n); }

    size_t move_first(size_t const src, size_t const dst)
    {
        auto const s = items()[src + _n];

        if (s < _n)
        {
            (void)move_node(s, dst + _n);
            return s;
        }

        return _n;
    }

    static multi_list* get(::etl::span<uint8_t> const mem)
    {
        return mem.reinterpret_as<multi_list>().data();
    }

    static multi_list* make(size_t n, size_t buckets, ::etl::span<uint8_t>& mem);

    multi_list(multi_list const&) = delete;

private:
    multi_list(size_t n, size_t buckets);

    idx_type* items() { return reinterpret_cast<idx_type*>(this + 1); }

    idx_type* backlinks() { return items() + _n + _buckets; }

    idx_type const* items() const { return reinterpret_cast<idx_type const*>(this + 1); }

    idx_type const* backlinks() const { return items() + _n + _buckets; }

    idx_type _n;
    idx_type _buckets;

    size_t move_node(size_t s, size_t dst);
};
} // namespace internal

template<typename C, typename T>
C collect(T const& v)
{
    C res;

    auto f = [&res](size_t const i) -> bool
    {
        res.push_back(i);
        return true;
    };
    v.iter(f);
    return res;
}

template<typename T>
size_t count(T const& v)
{
    size_t i = 0;
    v.iter(
        [&i](size_t const) -> bool
        {
            ++i;
            return true;
        });
    return i;
}

template<typename T>
bool is_empty(T const& v)
{
    bool empty = true;
    v.iter(
        [&empty](size_t const) -> bool
        {
            empty = false;
            return false;
        });
    return empty;
}

} // namespace shed
