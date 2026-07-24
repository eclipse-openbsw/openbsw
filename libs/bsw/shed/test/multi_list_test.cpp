/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "shed/table.h"

#include <etl/span.h>

#include <gmock/gmock.h>

using namespace testing;

alignas(alignof(::shed::internal::multi_list::idx_type)) static uint8_t
    ml_mem[::shed::internal::multi_list::memory_for(10, 3)];

using Ids = std::vector<size_t>;

TEST(A_multi_list, can_conditinally_transfer_indicies_between_buckets)
{
    ::etl::span<uint8_t> mem         = ::etl::span{ml_mem};
    ::shed::internal::multi_list& ms = *::shed::internal::multi_list::make(10, 3, mem);
    EXPECT_EQ(0U, mem.size());

    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre());
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    ms.move_if(0, 1, [](size_t i) { return ::shed::skip_if(i >= 4); });

    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(4, 5, 6, 7, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(0, 1, 2, 3));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    ms.move_if(1, 0, [](size_t /*i*/) { return ::shed::skip_if(false); });

    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre());
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_buckets(1, 2)),
        UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_buckets(0, 2)), UnorderedElementsAre());
    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_buckets(0, 1)), UnorderedElementsAre());

    ms.move_if(0, 1, [](size_t i) { return ::shed::skip_if(i % 3); });

    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(1, 2, 4, 5, 7, 8));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(0, 3, 6, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    ms.move_if(0, 2, [](size_t i) { return ::shed::skip_if(i % 2); });

    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(1, 5, 7));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(0, 3, 6, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre(2, 4, 8));

    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_buckets(1, 2)), UnorderedElementsAre(1, 5, 7));
    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_buckets(0, 2)), UnorderedElementsAre(0, 3, 6, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_buckets(0, 1)), UnorderedElementsAre(2, 4, 8));

    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(0)), UnorderedElementsAre(0, 2, 3, 4, 6, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_bucket(1)), UnorderedElementsAre(1, 2, 4, 5, 7, 8));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(2)), UnorderedElementsAre(0, 1, 3, 5, 6, 7, 9));
}

TEST(A_multi_list, can_iterate_items_that_are_not_in_a_bucket)
{
    ::etl::span<uint8_t> mem         = ::etl::span{ml_mem};
    ::shed::internal::multi_list& ms = *::shed::internal::multi_list::make(10, 3, mem);
    EXPECT_EQ(0U, mem.size());

    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_bucket(0)), UnorderedElementsAre());
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(1)),
        UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(2)),
        UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));

    EXPECT_EQ(0U, ::shed::count(ms.not_in_bucket(0)));
    EXPECT_EQ(10U, ::shed::count(ms.not_in_bucket(1)));
    EXPECT_EQ(10U, ::shed::count(ms.not_in_bucket(2)));

    ms.move_if(0, 1, [](size_t i) { return ::shed::skip_if(i >= 4); });

    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3));
    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_bucket(1)), UnorderedElementsAre(4, 5, 6, 7, 8, 9));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(2)),
        UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));

    ms.move_if(1, 0, [](size_t /*i*/) { return ::shed::skip_if(false); });

    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_bucket(0)), UnorderedElementsAre());
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(1)),
        UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(2)),
        UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));

    ms.move_if(0, 1, [](size_t i) { return ::shed::skip_if(i % 3); });

    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_bucket(0)), UnorderedElementsAre(0, 3, 6, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_bucket(1)), UnorderedElementsAre(1, 2, 4, 5, 7, 8));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(2)),
        UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));

    ms.move_if(0, 2, [](size_t i) { return ::shed::skip_if(i % 2); });

    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(0)), UnorderedElementsAre(0, 2, 3, 4, 6, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_bucket(1)), UnorderedElementsAre(1, 2, 4, 5, 7, 8));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(2)), UnorderedElementsAre(0, 1, 3, 5, 6, 7, 9));

    ms.move_if(1, 0, [](size_t /*i*/) { return ::shed::skip_if(false); });
    ms.move_if(2, 0, [](size_t /*i*/) { return ::shed::skip_if(false); });
    ms.move_if(0, 1, [](size_t i) { return ::shed::skip_if(i >= 1); });

    EXPECT_THAT(::shed::collect<Ids>(ms.not_in_bucket(0)), UnorderedElementsAre(0));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(1)), UnorderedElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.not_in_bucket(2)),
        UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(A_multi_list, can_move_a_single_element_between_buckets)
{
    ::etl::span<uint8_t> mem         = ::etl::span{ml_mem};
    ::shed::internal::multi_list& ms = *::shed::internal::multi_list::make(10, 3, mem);
    EXPECT_EQ(0U, mem.size());

    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre());
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    EXPECT_EQ(9U, (ms.move_first(0, 1)));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    EXPECT_EQ(9U, (ms.move_first(1, 2)));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre());
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre(9));

    EXPECT_EQ(8U, (ms.move_first(0, 2)));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre());
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre(8, 9));

    EXPECT_EQ(10U, (ms.move_first(1, 2)));
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre());
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre(8, 9));
}

TEST(A_multi_list, can_move_a_single_element_from_any_bucket)
{
    ::etl::span<uint8_t> mem         = ::etl::span{ml_mem};
    ::shed::internal::multi_list& ms = *::shed::internal::multi_list::make(10, 3, mem);
    EXPECT_EQ(0U, mem.size());

    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre());
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    ms.move_idx(4, 1);
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 5, 6, 7, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(4));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    // already in target bucket
    ms.move_idx(4, 1);
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(0, 1, 2, 3, 5, 6, 7, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(4));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    ms.move_idx(0, 1);
    EXPECT_THAT(
        ::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(1, 2, 3, 5, 6, 7, 8, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(0, 4));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    ms.move_idx(9, 1);
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(1, 2, 3, 5, 6, 7, 8));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(0, 4, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());

    ms.move_idx(7, 2);
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(1, 2, 3, 5, 6, 8));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(0, 4, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre(7));

    // bucket becomes empty
    ms.move_idx(7, 1);
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(0)), UnorderedElementsAre(1, 2, 3, 5, 6, 8));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(1)), UnorderedElementsAre(0, 4, 7, 9));
    EXPECT_THAT(::shed::collect<Ids>(ms.in_bucket(2)), UnorderedElementsAre());
}

/* This test ensures that unaligned memory will fail to create a multi_list
 */
TEST(multi_list, unaligned_memory_fails_to_create_list)
{
    alignas(alignof(max_align_t))
        uint8_t ml_unaligned_mem[::shed::internal::multi_list::memory_for(10, 3) + 1];
    auto mem                         = ::etl::span{ml_unaligned_mem}.subspan(1);
    auto const preMakeSize           = mem.size();
    ::shed::internal::multi_list* ms = ::shed::internal::multi_list::make(10, 3, mem);
    EXPECT_EQ(preMakeSize, mem.size());
    EXPECT_EQ(nullptr, ms);
}
