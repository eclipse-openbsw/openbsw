/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "core/mock/PduIdQueueMock.h"
#include "core/mock/PduStorageMock.h"
#include "core/mock/PduTxSinkMock.h"
#include "middleware/core/PduBrokerConnection.h"

#include <etl/array.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace middleware::core::test
{
namespace
{

using ::testing::_;
using ::testing::ElementsAreArray;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::Truly;

// PDU IDs come from the deployment: they are not sequential and need not start at 0.
constexpr uint32_t SMALL_ID   = 10U;
constexpr uint32_t MID_ID     = 77U;
constexpr uint32_t LARGE_ID   = 500U;
constexpr uint32_t UNKNOWN_ID = 11U;

constexpr size_t SMALL_SIZE = 4U;
constexpr size_t LARGE_SIZE = 8U;

StrictMock<PduStorageMock>* gStorage{nullptr};

// The PDU functions are plain function pointers, so they forward to the mock with their ID.
template<uint32_t ID>
::etl::span<uint8_t const> loadPdu(::etl::span<uint8_t> const out)
{
    return gStorage->load(ID, out);
}

template<uint32_t ID>
bool storePdu(::etl::span<uint8_t const> const in)
{
    return gStorage->store(ID, in);
}

// Same shape as the table generated from the deployment: sorted by PDU ID.
constexpr ::etl::array<PduAccess, 3U> PDU_TABLE
    = {{{SMALL_ID, &loadPdu<SMALL_ID>, &storePdu<SMALL_ID>},
        {MID_ID, &loadPdu<MID_ID>, &storePdu<MID_ID>},
        {LARGE_ID, &loadPdu<LARGE_ID>, &storePdu<LARGE_ID>}}};

class PduBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override { gStorage = &storage; }

    void TearDown() override { gStorage = nullptr; }

    // The ID is at the head of the queue and the queue is not empty.
    void expectHead(uint32_t const id)
    {
        EXPECT_CALL(idQueue, isEmpty()).WillOnce(Return(false));
        EXPECT_CALL(idQueue, peek()).WillOnce(Return(id));
    }

    void expectQueueEmpty() { EXPECT_CALL(idQueue, isEmpty()).WillOnce(Return(true)); }

    // The PDU is loaded into a buffer that fits the largest PDU and has the given content.
    void expectLoad(uint32_t const id, ::etl::span<uint8_t const> const content)
    {
        EXPECT_CALL(
            storage,
            load(
                id,
                Truly([](::etl::span<uint8_t> const& buffer)
                      { return buffer.size() == LARGE_SIZE; })))
            .WillOnce(Return(content));
    }

    StrictMock<PduStorageMock> storage;
    StrictMock<PduIdQueueMock> idQueue;
    StrictMock<PduTxSinkMock> txSink;
    PduBrokerConnection<PduIdQueueMock, LARGE_SIZE> connection{txSink, PDU_TABLE, idQueue};
    ::etl::array<uint8_t, SMALL_SIZE> const smallPayload{1U, 2U, 3U, 4U};
    ::etl::array<uint8_t, LARGE_SIZE> const largePayload{8U, 7U, 6U, 5U, 4U, 3U, 2U, 1U};
};

TEST_F(PduBrokerTest, ReceivedPduIsStoredInThePduOfThatId)
{
    EXPECT_CALL(storage, store(SMALL_ID, ElementsAreArray(smallPayload))).WillOnce(Return(true));

    EXPECT_TRUE(connection.onReceive(SMALL_ID, smallPayload));
}

TEST_F(PduBrokerTest, ReceivedPduIsRejectedWhenThePduDoesNotStoreIt)
{
    EXPECT_CALL(storage, store(SMALL_ID, _)).WillOnce(Return(false));

    EXPECT_FALSE(connection.onReceive(SMALL_ID, smallPayload));
}

TEST_F(PduBrokerTest, ReceivedPduIsFoundForEveryIdInTheTable)
{
    EXPECT_CALL(storage, store(MID_ID, _)).WillOnce(Return(true));
    EXPECT_CALL(storage, store(LARGE_ID, ElementsAreArray(largePayload))).WillOnce(Return(true));
    EXPECT_CALL(storage, store(SMALL_ID, _)).WillOnce(Return(true));

    EXPECT_TRUE(connection.onReceive(MID_ID, smallPayload));
    EXPECT_TRUE(connection.onReceive(LARGE_ID, largePayload));
    EXPECT_TRUE(connection.onReceive(SMALL_ID, smallPayload));
}

TEST_F(PduBrokerTest, IdsThatAreNotInTheTableAreRejectedWithoutTouchingAnyPdu)
{
    // The storage mock is strict: any call to it fails the test.
    for (uint32_t const id :
         {0U, SMALL_ID - 1U, UNKNOWN_ID, MID_ID + 1U, LARGE_ID - 1U, LARGE_ID + 1U, 0xFFFFFFFFU})
    {
        EXPECT_FALSE(connection.onReceive(id, smallPayload)) << "id " << id;
    }
}

TEST_F(PduBrokerTest, NothingIsTransmittedWhenQueueIsEmpty)
{
    expectQueueEmpty();

    connection.processTransmissions();
}

TEST_F(PduBrokerTest, CommittedPdusAreTransmittedInCommitOrder)
{
    InSequence const sequence;
    expectHead(LARGE_ID);
    expectLoad(LARGE_ID, largePayload);
    EXPECT_CALL(txSink, submit(LARGE_ID, ElementsAreArray(largePayload)));
    EXPECT_CALL(idQueue, advance());
    expectHead(SMALL_ID);
    expectLoad(SMALL_ID, smallPayload);
    EXPECT_CALL(txSink, submit(SMALL_ID, ElementsAreArray(smallPayload)));
    EXPECT_CALL(idQueue, advance());
    expectQueueEmpty();

    connection.processTransmissions();
}

TEST_F(PduBrokerTest, PduIsLoadedWhenItsIdIsDrainedAndNotBefore)
{
    ::etl::array<uint8_t, SMALL_SIZE> const updated{9U, 9U, 9U, 9U};

    // Nothing is loaded until the ID reaches the head of the queue, so the latest content is sent.
    InSequence const sequence;
    expectHead(SMALL_ID);
    expectLoad(SMALL_ID, updated);
    EXPECT_CALL(txSink, submit(SMALL_ID, ElementsAreArray(updated)));
    EXPECT_CALL(idQueue, advance());
    expectQueueEmpty();

    connection.processTransmissions();
}

TEST_F(PduBrokerTest, UnknownIdsAreDiscarded)
{
    InSequence const sequence;
    // Nothing is loaded or transmitted for the unknown ID, but it leaves the queue.
    expectHead(UNKNOWN_ID);
    EXPECT_CALL(idQueue, advance());
    expectHead(SMALL_ID);
    expectLoad(SMALL_ID, smallPayload);
    EXPECT_CALL(txSink, submit(SMALL_ID, ElementsAreArray(smallPayload)));
    EXPECT_CALL(idQueue, advance());
    expectQueueEmpty();

    connection.processTransmissions();

    EXPECT_EQ(connection.getUnknownTxIds(), 1U);
}

} // namespace
} // namespace middleware::core::test
