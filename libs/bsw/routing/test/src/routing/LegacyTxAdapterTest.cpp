/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/LegacyTxAdapter.h"

#include "routing/definition.h"

#include <etl/optional.h>
#include <io/IWriterMock.h>
#include <io/MemoryQueue.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

template<size_t CAPACITY = 1024, size_t MAX_ELEMENT_SIZE = 128>
struct LegacyTxAdapterTest
{
    using Queue  = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE>;
    using Reader = ::io::MemoryQueueReader<Queue>;
    using Writer = ::io::MemoryQueueWriter<Queue>;

    LegacyTxAdapterTest(::etl::span<::routing::Definition> defs)
    : txQueue()
    , txReader(txQueue)
    , txWriter(txQueue)
    , unknownMessageIdCounter(0)
    , memAllocationFailureCounter(0)
    , errorHandler(
          ::routing::ErrorHandler::Function::create<
              LegacyTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE>,
              &LegacyTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE>::handleError>(*this),
          0U)
    {
        ::routing::load(txAdapterTable, defs, 0, txAdapterTableMem);
        (void)txAdapter.emplace(txWriter, txAdapterTable, errorHandler);
    }

    void handleError(::routing::ErrorHandler::StatusCode const statusCode, uint8_t, uint32_t)
    {
        switch (statusCode)
        {
            case ::routing::ErrorHandler::StatusCode::UNKNOWN_MESSAGE_ID:
                ++unknownMessageIdCounter;
                break;
            case ::routing::ErrorHandler::StatusCode::MEM_ALLOCATION_FAILURE:
                ++memAllocationFailureCounter;
                break;
            default: break;
        }
    }

    Queue txQueue;
    Reader txReader;
    Writer txWriter;

    ::routing::TxAdapterTableMem txAdapterTableMem;
    ::routing::TxAdapterTable txAdapterTable;

    uint32_t unknownMessageIdCounter;
    uint32_t memAllocationFailureCounter;

    ::routing::ErrorHandler errorHandler;

    ::etl::optional<::routing::LegacyTxAdapter> txAdapter;
};

::etl::span<uint8_t> allocatePdu(
    uint32_t const id, size_t const size, ::io::IWriter& writer, uint8_t const pattern = 0xAB)
{
    auto pdu = writer.allocate(size);
    if (!pdu.empty())
    {
        auto data                       = pdu;
        data.take<::etl::be_uint32_t>() = id;
        auto& length                    = data.take<::etl::be_uint32_t>();
        length                          = data.size();
        ::etl::fill(data.begin(), data.end(), pattern);
    }
    return pdu;
}

void writePdu(
    uint32_t const id, size_t const size, ::io::IWriter& writer, uint8_t const pattern = 0xAB)
{
    auto pdu = allocatePdu(id, size, writer, pattern);
    if (!pdu.empty())
    {
        writer.commit();
    }
}

/**
 * \desc
 * The max_size function returns the correct value.
 */
TEST(LegacyTxAdapterTest, max_size)
{
    constexpr size_t CAPACITY         = 128;
    constexpr size_t MAX_ELEMENT_SIZE = 16;

    LegacyTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest({});
    auto& txAdapter = *txAdapterTest.txAdapter;
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.maxSize());
}

/**
 * \desc
 * Sending an outgoing message identical to the PDU works
 */
TEST(LegacyTxAdapterTest, identical_pdu)
{
    constexpr uint8_t MESSAGE_ID      = 0x01;
    constexpr uint32_t HEADER_LENGTH  = 8;
    constexpr uint32_t PDU_OFFSET     = 0;
    constexpr uint32_t MESSAGE_LENGTH = 8;
    constexpr uint8_t PATTERN         = 0xAB;

    ::routing::Definition defs[]
        = {::routing::Definition().out(0, MESSAGE_ID, MESSAGE_LENGTH, PDU_OFFSET)};
    LegacyTxAdapterTest<> txAdapterTest(defs);
    auto adapter = *txAdapterTest.txAdapter;
    auto reader  = txAdapterTest.txReader;

    writePdu(MESSAGE_ID, HEADER_LENGTH + MESSAGE_LENGTH, adapter);

    auto res = reader.peek();
    EXPECT_EQ(HEADER_LENGTH + MESSAGE_LENGTH, res.size());
    EXPECT_EQ(1, adapter.pduCounter());
    EXPECT_EQ(HEADER_LENGTH + MESSAGE_LENGTH, adapter.byteCounter());
    EXPECT_EQ(0, adapter.droppedPduCounter());
    EXPECT_EQ(0, adapter.droppedByteCounter());

    uint32_t id = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(MESSAGE_ID, id);
    uint32_t l = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(MESSAGE_LENGTH, l);
    ::etl::array<uint8_t, MESSAGE_LENGTH> expectedPayload;
    expectedPayload.fill(PATTERN);
    EXPECT_THAT(res.first(MESSAGE_LENGTH), ElementsAreArray(expectedPayload));
}

/**
 * \desc
 * Sending an outgoing message with space in front of the PDU works (offset)
 */
TEST(LegacyTxAdapterTest, space_before)
{
    constexpr uint8_t MESSAGE_ID      = 0x01;
    constexpr uint32_t HEADER_LENGTH  = 8;
    constexpr uint32_t PDU_OFFSET     = 4;
    constexpr uint32_t PDU_LENGTH     = 4;
    constexpr uint32_t MESSAGE_LENGTH = PDU_OFFSET + PDU_LENGTH;
    constexpr uint8_t PATTERN         = 0xAB;

    ::routing::Definition defs[]
        = {::routing::Definition().out(0, MESSAGE_ID, MESSAGE_LENGTH, PDU_OFFSET)};
    LegacyTxAdapterTest<> txAdapterTest(defs);
    auto adapter = *txAdapterTest.txAdapter;
    auto reader  = txAdapterTest.txReader;

    writePdu(MESSAGE_ID, HEADER_LENGTH + PDU_LENGTH, adapter);

    auto res = reader.peek();
    EXPECT_EQ(HEADER_LENGTH + MESSAGE_LENGTH, res.size());
    EXPECT_EQ(1, adapter.pduCounter());
    EXPECT_EQ(HEADER_LENGTH + MESSAGE_LENGTH, adapter.byteCounter());
    EXPECT_EQ(0, adapter.droppedPduCounter());
    EXPECT_EQ(0, adapter.droppedByteCounter());

    uint32_t id = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(MESSAGE_ID, id);
    uint32_t l = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(MESSAGE_LENGTH, l);
    ::etl::array<uint8_t, PDU_OFFSET> expectedFiller;
    expectedFiller.fill(0xFF);
    EXPECT_THAT(res.take<uint8_t>(PDU_OFFSET), ElementsAreArray(expectedFiller));
    ::etl::array<uint8_t, PDU_LENGTH> expectedPayload;
    expectedPayload.fill(PATTERN);
    EXPECT_THAT(res, ElementsAreArray(expectedPayload));
}

/**
 * \desc
 * Sending an outgoing message larger than the PDU works (padding)
 */
TEST(LegacyTxAdapterTest, space_after)
{
    constexpr uint8_t MESSAGE_ID      = 0x01;
    constexpr uint32_t HEADER_LENGTH  = 8;
    constexpr uint32_t PDU_OFFSET     = 0;
    constexpr uint32_t PDU_LENGTH     = 8;
    constexpr uint32_t MESSAGE_LENGTH = PDU_LENGTH + 8;
    constexpr uint8_t PATTERN         = 0xAB;

    ::routing::Definition defs[]
        = {::routing::Definition().out(0, MESSAGE_ID, MESSAGE_LENGTH, PDU_OFFSET)};
    LegacyTxAdapterTest<> txAdapterTest(defs);
    auto adapter = *txAdapterTest.txAdapter;
    auto reader  = txAdapterTest.txReader;

    writePdu(MESSAGE_ID, HEADER_LENGTH + PDU_LENGTH, adapter);

    auto res = reader.peek();
    EXPECT_EQ(HEADER_LENGTH + MESSAGE_LENGTH, res.size());
    EXPECT_EQ(1, adapter.pduCounter());
    EXPECT_EQ(HEADER_LENGTH + MESSAGE_LENGTH, adapter.byteCounter());
    EXPECT_EQ(0, adapter.droppedPduCounter());
    EXPECT_EQ(0, adapter.droppedByteCounter());

    uint32_t id = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(MESSAGE_ID, id);
    uint32_t l = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(MESSAGE_LENGTH, l);
    ::etl::array<uint8_t, PDU_LENGTH> expectedPayload;
    expectedPayload.fill(PATTERN);
    EXPECT_THAT(res.take<uint8_t>(PDU_LENGTH), ElementsAreArray(expectedPayload));
    ::etl::array<uint8_t, MESSAGE_LENGTH - (PDU_LENGTH + PDU_OFFSET)> expectedFiller;
    expectedFiller.fill(0xFF);
    EXPECT_THAT(res, ElementsAreArray(expectedFiller));
}

/**
 * \desc
 * Sending a small PDU with an offset works (padding + offset)
 */
TEST(LegacyTxAdapterTest, space_before_after)
{
    constexpr uint8_t MESSAGE_ID      = 0x01;
    constexpr uint32_t HEADER_LENGTH  = 8;
    constexpr uint32_t PDU_OFFSET     = 2;
    constexpr uint32_t PDU_LENGTH     = 4;
    constexpr uint32_t MESSAGE_LENGTH = 16;
    constexpr uint8_t PATTERN         = 0xAB;

    ::routing::Definition defs[]
        = {::routing::Definition().out(0, MESSAGE_ID, MESSAGE_LENGTH, PDU_OFFSET)};
    LegacyTxAdapterTest<> txAdapterTest(defs);
    auto adapter = *txAdapterTest.txAdapter;
    auto reader  = txAdapterTest.txReader;

    writePdu(MESSAGE_ID, HEADER_LENGTH + PDU_LENGTH, adapter);

    auto res = reader.peek();
    EXPECT_EQ(HEADER_LENGTH + MESSAGE_LENGTH, res.size());
    EXPECT_EQ(1, adapter.pduCounter());
    EXPECT_EQ(HEADER_LENGTH + MESSAGE_LENGTH, adapter.byteCounter());
    EXPECT_EQ(0, adapter.droppedPduCounter());
    EXPECT_EQ(0, adapter.droppedByteCounter());

    uint32_t id = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(MESSAGE_ID, id);
    uint32_t l = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(MESSAGE_LENGTH, l);
    ::etl::array<uint8_t, PDU_OFFSET> paddingBefore;
    paddingBefore.fill(0xFF);
    EXPECT_THAT(res.take<uint8_t>(PDU_OFFSET), ElementsAreArray(paddingBefore));
    ::etl::array<uint8_t, PDU_LENGTH> expectedPayload;
    expectedPayload.fill(PATTERN);
    EXPECT_THAT(res.take<uint8_t>(PDU_LENGTH), ElementsAreArray(expectedPayload));
    ::etl::array<uint8_t, MESSAGE_LENGTH - (PDU_LENGTH + PDU_OFFSET)> paddingAfter;
    paddingAfter.fill(0xFF);
    EXPECT_THAT(res, ElementsAreArray(paddingAfter));
}

/**
 * \desc
 * Sending a small PDU multiple times in a row works.
 * */
TEST(LegacyTxAdapterTest, send_multiple_times)
{
    constexpr uint8_t MESSAGE_ID      = 0x01;
    constexpr uint32_t HEADER_LENGTH  = 8;
    constexpr uint32_t PDU_OFFSET     = 2;
    constexpr uint32_t PDU_LENGTH     = 4;
    constexpr uint32_t MESSAGE_LENGTH = 16;
    constexpr uint8_t PATTERN         = 0xAB;
    constexpr size_t ITERATIONS       = 20;

    ::routing::Definition defs[]
        = {::routing::Definition().out(0, MESSAGE_ID, MESSAGE_LENGTH, PDU_OFFSET)};
    LegacyTxAdapterTest<> txAdapterTest(defs);
    auto adapter = *txAdapterTest.txAdapter;
    auto reader  = txAdapterTest.txReader;

    for (size_t i = 0; i < ITERATIONS; i++)
    {
        writePdu(MESSAGE_ID, HEADER_LENGTH + PDU_LENGTH, adapter);

        EXPECT_EQ(i + 1, adapter.pduCounter());
        EXPECT_EQ((i + 1) * (HEADER_LENGTH + MESSAGE_LENGTH), adapter.byteCounter());
        EXPECT_EQ(0, adapter.droppedPduCounter());
        EXPECT_EQ(0, adapter.droppedByteCounter());
    }

    for (size_t i = 0; i < ITERATIONS; i++)
    {
        auto res = reader.peek();
        EXPECT_EQ(HEADER_LENGTH + MESSAGE_LENGTH, res.size());

        uint32_t id = res.take<::etl::be_uint32_t>();
        EXPECT_EQ(MESSAGE_ID, id);
        uint32_t l = res.take<::etl::be_uint32_t>();
        EXPECT_EQ(MESSAGE_LENGTH, l);
        ::etl::array<uint8_t, PDU_OFFSET> paddingBefore;
        paddingBefore.fill(0xFF);
        EXPECT_THAT(res.take<uint8_t>(PDU_OFFSET), ElementsAreArray(paddingBefore));
        ::etl::array<uint8_t, PDU_LENGTH> expectedPayload;
        expectedPayload.fill(PATTERN);
        EXPECT_THAT(res.take<uint8_t>(PDU_LENGTH), ElementsAreArray(expectedPayload));
        ::etl::array<uint8_t, MESSAGE_LENGTH - (PDU_LENGTH + PDU_OFFSET)> paddingAfter;
        paddingAfter.fill(0xFF);
        EXPECT_THAT(res, ElementsAreArray(paddingAfter));

        reader.release();
    }
}

/**
 * \desc
 * PDU is dropped on allocation if its size is greater than max element size of queue.
 */
TEST(LegacyTxAdapterTest, drop_pdu_on_allocation_pdu_is_too_big)
{
    constexpr size_t CAPACITY         = 64;
    constexpr size_t MAX_ELEMENT_SIZE = 16;

    ::routing::Definition defs[] = {::routing::Definition().out(0, 0x1, 8, 0)};
    LegacyTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest(defs);
    auto& txAdapter = *txAdapterTest.txAdapter;

    auto pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);
    pdu      = allocatePdu(0x1, MAX_ELEMENT_SIZE + 1, txAdapter);
    txAdapter.commit();

    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(0, txAdapter.pduCounter());
    EXPECT_EQ(0, txAdapter.byteCounter());
    EXPECT_EQ(1, txAdapter.droppedPduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE + 1, txAdapter.droppedByteCounter());
    EXPECT_EQ(1, txAdapter.oversizedPdus());
    EXPECT_EQ(0, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(0, txAdapter.unknownMessagePdus());
    EXPECT_EQ(0, txAdapter.failedMemMovePdus());
}

/**
 * \desc
 * PDU is dropped on allocation if queue is full.
 */
TEST(LegacyTxAdapterTest, drop_pdu_on_allocation_queue_is_full)
{
    constexpr size_t MAX_ELEMENT_SIZE = 16;
    constexpr size_t CAPACITY         = 32;

    ::routing::Definition defs[] = {::routing::Definition().out(0, 0x1, 8, 0)};
    LegacyTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest(defs);
    auto& txAdapter = *txAdapterTest.txAdapter;
    writePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);
    auto pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE - 1, txAdapter);

    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(1, txAdapter.pduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.byteCounter());
    EXPECT_EQ(1, txAdapter.droppedPduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE - 1, txAdapter.droppedByteCounter());
    EXPECT_EQ(0, txAdapter.oversizedPdus());
    EXPECT_EQ(1, txAdapterTest.memAllocationFailureCounter);
    EXPECT_EQ(1, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(0, txAdapter.unknownMessagePdus());
    EXPECT_EQ(0, txAdapter.failedMemMovePdus());
}

/**
 * \desc
 * If the queue is full twice in a row, the error handling is only called once.
 */
TEST(LegacyTxAdapterTest, multiple_drops_one_error_handling)
{
    constexpr size_t MAX_ELEMENT_SIZE = 16;
    constexpr size_t CAPACITY         = 32;

    ::routing::Definition defs[] = {::routing::Definition().out(0, 0x1, 8, 0)};
    LegacyTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest(defs);
    auto& txAdapter = *txAdapterTest.txAdapter;
    writePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);

    auto pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE - 1, txAdapter);
    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(1, txAdapter.pduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.byteCounter());
    EXPECT_EQ(1, txAdapter.droppedPduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE - 1, txAdapter.droppedByteCounter());
    EXPECT_EQ(0, txAdapter.oversizedPdus());
    EXPECT_EQ(1, txAdapterTest.memAllocationFailureCounter);
    EXPECT_EQ(1, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(0, txAdapter.unknownMessagePdus());
    EXPECT_EQ(0, txAdapter.failedMemMovePdus());

    pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE - 1, txAdapter);
    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(1, txAdapter.pduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.byteCounter());
    EXPECT_EQ(2, txAdapter.droppedPduCounter());
    EXPECT_EQ((MAX_ELEMENT_SIZE - 1) * 2, txAdapter.droppedByteCounter());
    EXPECT_EQ(0, txAdapter.oversizedPdus());
    EXPECT_EQ(1, txAdapterTest.memAllocationFailureCounter); // only called once
    EXPECT_EQ(2, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(0, txAdapter.unknownMessagePdus());
    EXPECT_EQ(0, txAdapter.failedMemMovePdus());

    // Succesfully allocating a PDU then failing counts again
    auto reader = txAdapterTest.txReader;
    reader.peek();
    reader.release();
    writePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);

    pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE - 1, txAdapter);
    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(2, txAdapter.pduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE * 2, txAdapter.byteCounter());
    EXPECT_EQ(3, txAdapter.droppedPduCounter());
    EXPECT_EQ((MAX_ELEMENT_SIZE - 1) * 3, txAdapter.droppedByteCounter());
    EXPECT_EQ(0, txAdapter.oversizedPdus());
    EXPECT_EQ(2, txAdapterTest.memAllocationFailureCounter); // this one is counted
    EXPECT_EQ(3, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(0, txAdapter.unknownMessagePdus());
    EXPECT_EQ(0, txAdapter.failedMemMovePdus());
}

/**
 * \desc
 * PDU is dropped on commit if message ID is unknown.
 */
TEST(LegacyTxAdapterTest, drop_pdu_on_commit_unknown_message_id)
{
    constexpr size_t CAPACITY         = 64;
    constexpr size_t MAX_ELEMENT_SIZE = 32;

    ::routing::Definition defs[] = {::routing::Definition().out(0, 0x1, 8, 0)};
    LegacyTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest(defs);
    auto& txAdapter = *txAdapterTest.txAdapter;
    auto& txReader  = txAdapterTest.txReader;

    writePdu(0x10, 16, txAdapter);

    EXPECT_EQ(0, txReader.peek().size());
    EXPECT_EQ(0, txAdapter.pduCounter());
    EXPECT_EQ(0, txAdapter.byteCounter());
    EXPECT_EQ(1, txAdapter.droppedPduCounter());
    EXPECT_EQ(16, txAdapter.droppedByteCounter());
    EXPECT_EQ(1, txAdapterTest.unknownMessageIdCounter);
    EXPECT_EQ(0, txAdapter.oversizedPdus());
    EXPECT_EQ(0, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(1, txAdapter.unknownMessagePdus());
    EXPECT_EQ(0, txAdapter.failedMemMovePdus());
}

/**
 * \desc
 * PDU is dropped on commit if outgoing message size is greater than max element size of queue.
 */
TEST(LegacyTxAdapterTest, drop_pdu_on_commit_output_message_is_too_big)
{
    constexpr size_t CAPACITY         = 64;
    constexpr size_t MAX_ELEMENT_SIZE = 16;

    ::routing::Definition defs[] = {::routing::Definition().out(0, 0x1, 16, 0)};
    LegacyTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest(defs);
    auto& txAdapter = *txAdapterTest.txAdapter;
    auto& txReader  = txAdapterTest.txReader;

    writePdu(0x1, 16, txAdapter);

    EXPECT_EQ(0, txReader.peek().size());
    EXPECT_EQ(0, txAdapter.pduCounter());
    EXPECT_EQ(0, txAdapter.byteCounter());
    EXPECT_EQ(1, txAdapter.droppedPduCounter());
    EXPECT_EQ(24, txAdapter.droppedByteCounter());
    EXPECT_EQ(0, txAdapter.oversizedPdus());
    EXPECT_EQ(1, txAdapterTest.memAllocationFailureCounter);
    EXPECT_EQ(1, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(0, txAdapter.unknownMessagePdus());
    EXPECT_EQ(0, txAdapter.failedMemMovePdus());
}

/**
 * \desc
 * PDU is dropped on commit if PDU offset is equal to or greater than message length.
 */
TEST(LegacyTxAdapterTest, drop_pdu_on_commit_invalid_offset)
{
    constexpr size_t CAPACITY         = 64;
    constexpr size_t MAX_ELEMENT_SIZE = 16;

    ::routing::Definition defs[]
        = {::routing::Definition().out(0, 0x1, 4, 8), ::routing::Definition().out(0, 0x2, 8, 8)};
    LegacyTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest(defs);
    auto& txAdapter                    = *txAdapterTest.txAdapter;
    auto& txReader                     = txAdapterTest.txReader;
    uint32_t expectedDroppedPduCounter = 0U, expectedDroppedByteCounter = 0U;

    writePdu(0x1, 16, txAdapter);

    expectedDroppedPduCounter += 1;
    expectedDroppedByteCounter += 12;

    EXPECT_EQ(0, txReader.peek().size());
    EXPECT_EQ(0, txAdapter.pduCounter());
    EXPECT_EQ(0, txAdapter.byteCounter());
    EXPECT_EQ(expectedDroppedPduCounter, txAdapter.droppedPduCounter());
    EXPECT_EQ(expectedDroppedByteCounter, txAdapter.droppedByteCounter());
    EXPECT_EQ(0, txAdapter.oversizedPdus());
    EXPECT_EQ(0, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(0, txAdapter.unknownMessagePdus());
    EXPECT_EQ(expectedDroppedPduCounter, txAdapter.failedMemMovePdus());

    writePdu(0x2, 16, txAdapter);

    expectedDroppedPduCounter += 1;
    expectedDroppedByteCounter += 16;

    EXPECT_EQ(0, txReader.peek().size());
    EXPECT_EQ(0, txAdapter.pduCounter());
    EXPECT_EQ(0, txAdapter.byteCounter());
    EXPECT_EQ(expectedDroppedPduCounter, txAdapter.droppedPduCounter());
    EXPECT_EQ(expectedDroppedByteCounter, txAdapter.droppedByteCounter());
    EXPECT_EQ(0, txAdapter.oversizedPdus());
    EXPECT_EQ(0, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(0, txAdapter.unknownMessagePdus());
    EXPECT_EQ(expectedDroppedPduCounter, txAdapter.failedMemMovePdus());

    EXPECT_EQ(0, txReader.peek().size());
    EXPECT_EQ(0, txAdapter.pduCounter());
    EXPECT_EQ(0, txAdapter.byteCounter());
    EXPECT_EQ(expectedDroppedPduCounter, txAdapter.droppedPduCounter());
    EXPECT_EQ(expectedDroppedByteCounter, txAdapter.droppedByteCounter());
    EXPECT_EQ(0, txAdapter.oversizedPdus());
    EXPECT_EQ(0, txAdapter.failedMemAllocPdus());
    EXPECT_EQ(0, txAdapter.unknownMessagePdus());
    EXPECT_EQ(expectedDroppedPduCounter, txAdapter.failedMemMovePdus());
}

/**
 * \desc
 * After allocating some data, calling commit twice should write it to the queue only once.
 */
TEST(LegacyTxAdapterTest, commit_twice)
{
    ::routing::Definition defs[] = {::routing::Definition().out(0, 1, 8, 0)};
    LegacyTxAdapterTest<> txAdapterTest(defs);
    auto adapter = *txAdapterTest.txAdapter;
    auto reader  = txAdapterTest.txReader;

    allocatePdu(1, 16, adapter);
    adapter.commit();
    adapter.commit();

    EXPECT_EQ(16, reader.peek().size());
    EXPECT_EQ(1, adapter.pduCounter());
    EXPECT_EQ(16, adapter.byteCounter());

    reader.release();

    EXPECT_EQ(0, reader.peek().size());
}

/**
 * \desc
 *  Flush triggers the output writer's flush method.
 */
TEST(LegacyTxAdapterTest, flush)
{
    ::io::IWriterMock writerMock;
    ::routing::TxAdapterTable txAdapterTable;

    EXPECT_CALL(writerMock, flush()).Times(1);

    ::routing::LegacyTxAdapter txAdapter(writerMock, txAdapterTable, {});

    txAdapter.flush();
}

} // namespace
