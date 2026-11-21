/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/core/MiddlewarePdu.h"
#include "middleware/core/PduAccess.h"

#include <etl/array.h>
#include <etl/span.h>

#include <gtest/gtest.h>

#include <new>

namespace middleware::core::test
{
namespace
{

constexpr size_t PDU_SIZE = 4U;
constexpr uint32_t PDU_ID = 77U;

MiddlewarePdu<PDU_SIZE> firstSlot;
MiddlewarePdu<PDU_SIZE> secondSlot;
MiddlewarePdu<PDU_SIZE>* currentSlot = &firstSlot;

MiddlewarePdu<PDU_SIZE>* slot() { return currentSlot; }

constexpr PduAccess ACCESS = makePduAccess<PDU_SIZE, &slot>(PDU_ID);

class PduSlotAccessTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        firstSlot.~MiddlewarePdu();
        new (&firstSlot) MiddlewarePdu<PDU_SIZE>();
        secondSlot.~MiddlewarePdu();
        new (&secondSlot) MiddlewarePdu<PDU_SIZE>();
        currentSlot = &firstSlot;
    }

    static bool store(::etl::array<uint8_t, PDU_SIZE> const& payload)
    {
        return ACCESS.store(::etl::span<uint8_t const>(payload));
    }
};

TEST_F(PduSlotAccessTest, EntryHoldsTheId) { EXPECT_EQ(ACCESS.id, PDU_ID); }

TEST_F(PduSlotAccessTest, StoredPayloadIsLoadedAndTheRestOfTheBufferIsUntouched)
{
    ::etl::array<uint8_t, PDU_SIZE> const payload{1U, 2U, 3U, 4U};
    ::etl::array<uint8_t, PDU_SIZE + 2U> buffer{};
    buffer.fill(0xAAU);

    ASSERT_TRUE(store(payload));
    auto const loaded = ACCESS.load(::etl::span<uint8_t>(buffer));

    ASSERT_EQ(loaded.size(), PDU_SIZE);
    EXPECT_TRUE(::etl::equal(loaded.begin(), loaded.end(), payload.begin()));
    EXPECT_EQ(buffer[PDU_SIZE], 0xAAU);
    EXPECT_EQ(buffer[PDU_SIZE + 1U], 0xAAU);
}

TEST_F(PduSlotAccessTest, StoreRejectsAWrongSizeAndKeepsThePdu)
{
    ::etl::array<uint8_t, PDU_SIZE> const payload{1U, 2U, 3U, 4U};
    ::etl::array<uint8_t, PDU_SIZE + 1U> const longer{9U, 9U, 9U, 9U, 9U};
    ASSERT_TRUE(store(payload));

    EXPECT_FALSE(ACCESS.store(::etl::span<uint8_t const>()));
    EXPECT_FALSE(ACCESS.store(::etl::span<uint8_t const>(longer.data(), PDU_SIZE - 1U)));
    EXPECT_FALSE(ACCESS.store(::etl::span<uint8_t const>(longer)));

    ::etl::array<uint8_t, PDU_SIZE> buffer{};
    auto const loaded = ACCESS.load(::etl::span<uint8_t>(buffer));
    EXPECT_TRUE(::etl::equal(loaded.begin(), loaded.end(), payload.begin()));
}

TEST_F(PduSlotAccessTest, TheSlotIsResolvedOnEveryCall)
{
    ::etl::array<uint8_t, PDU_SIZE> const payload{5U, 6U, 7U, 8U};
    ASSERT_TRUE(store(payload));

    currentSlot = &secondSlot;
    ::etl::array<uint8_t, PDU_SIZE> buffer{};
    auto const loaded = ACCESS.load(::etl::span<uint8_t>(buffer));
    EXPECT_EQ(loaded[0], 0xFFU) << "A PDU that was never stored holds all ones";

    ::etl::array<uint8_t, PDU_SIZE> const other{9U, 9U, 9U, 9U};
    ASSERT_TRUE(store(other));
    currentSlot      = &firstSlot;
    auto const first = ACCESS.load(::etl::span<uint8_t>(buffer));
    EXPECT_TRUE(::etl::equal(first.begin(), first.end(), payload.begin()));
}

} // namespace
} // namespace middleware::core::test
