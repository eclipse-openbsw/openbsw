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

#include <etl/array.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "concurrency/mock/ConcurrencyMock.h"

namespace middleware::core::test
{

using ::testing::Exactly;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Throw;

namespace
{
constexpr size_t PDU_SIZE = 4U;

// Thrown by platform hooks to leave an otherwise endless wait in load().
struct HookAborted
{};
} // namespace

class MiddlewarePduTest : public ::testing::Test
{
public:
    using Buffer = etl::array<uint8_t, PDU_SIZE>;

    void SetUp() final { concurrency::test::setConcurrencyMock(&_concurrencyMock); }

    void TearDown() final { concurrency::test::unsetConcurrencyMock(); }

protected:
    // The first fence of store() runs after the counter turned odd and before the data write.
    void EXPECT_READER_TO_SPIN_DURING_STORE()
    {
        EXPECT_CALL(_concurrencyMock, atomicThreadFence())
            .WillOnce(Invoke(
                [this]()
                {
                    EXPECT_THROW(_pdu.load(_buffer), HookAborted);
                    EXPECT_EQ(_buffer, Buffer{});
                }))
            .WillRepeatedly(Return());
        EXPECT_CALL(_concurrencyMock, spinLockOptimization()).WillOnce(Throw(HookAborted{}));
    }

    // The second fence of load() runs after the copy, so a store there tears the snapshot.
    void EXPECT_STORE_TO_TEAR_NEXT_LOAD(Buffer const& newData)
    {
        EXPECT_CALL(_concurrencyMock, atomicThreadFence())
            .WillOnce(Return())
            .WillOnce(Invoke([this, &newData]() { _pdu.store(newData); }))
            .WillRepeatedly(Return());
        _concurrencyMock.EXPECT_SPIN_CALLS(Exactly(1));
    }

    NiceMock<concurrency::test::ConcurrencyMock> _concurrencyMock{};
    MiddlewarePdu<PDU_SIZE> _pdu{};
    Buffer _buffer{};
};

TEST_F(MiddlewarePduTest, LoadBeforeFirstStoreReturnsAllBitsSetDataWithSequenceZero)
{
    _buffer.fill(0xDEU);

    EXPECT_EQ(_pdu.load(_buffer), 0U);
    EXPECT_THAT(_buffer, ::testing::Each(0xFFU));
}

TEST_F(MiddlewarePduTest, EachStoreAdvancesSequenceByTwoAndLoadReturnsLatestData)
{
    for (uint8_t i = 1U; i <= 3U; ++i)
    {
        Buffer const data{i, 2U, 3U, 4U};

        _pdu.store(data);

        EXPECT_EQ(_pdu.load(_buffer), 2U * i);
        EXPECT_EQ(_buffer, data);
    }
}

TEST_F(MiddlewarePduTest, StoreAndUncontendedLoadUseTwoFencesEachAndNeverSpin)
{
    _concurrencyMock.EXPECT_FENCE_CALLS(Exactly(4));
    _concurrencyMock.EXPECT_SPIN_CALLS(Exactly(0));

    Buffer const data{1U, 2U, 3U, 4U};

    _pdu.store(data);
    _pdu.load(_buffer);
}

TEST_F(MiddlewarePduTest, LoadSpinsWhileWriteIsInProgress)
{
    Buffer const data{1U, 2U, 3U, 4U};
    EXPECT_READER_TO_SPIN_DURING_STORE();

    _pdu.store(data);

    EXPECT_EQ(_pdu.load(_buffer), 2U);
    EXPECT_EQ(_buffer, data);
}

// Covers the unwinding edge of the retry loop, which gcov counts as a branch.
TEST_F(MiddlewarePduTest, LoadPropagatesExceptionRaisedByFence)
{
    EXPECT_CALL(_concurrencyMock, atomicThreadFence()).WillOnce(Throw(HookAborted{}));

    EXPECT_THROW(_pdu.load(_buffer), HookAborted);
}

TEST_F(MiddlewarePduTest, LoadRetriesWhenWriteOverlapsCopy)
{
    Buffer const oldData{1U, 2U, 3U, 4U};
    Buffer const newData{5U, 6U, 7U, 8U};
    _pdu.store(oldData);
    EXPECT_STORE_TO_TEAR_NEXT_LOAD(newData);

    EXPECT_EQ(_pdu.load(_buffer), 4U);
    EXPECT_EQ(_buffer, newData);
}

} // namespace middleware::core::test
