/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/PduTransportTxAdapter.h"

#include <etl/vector.h>
#include <io/IWriterMock.h>
#include <io/MemoryQueue.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

template<size_t CAPACITY = 1024, size_t MAX_ELEMENT_SIZE = 128>
struct PduTransportTxAdapterTest
{
    using Queue  = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE>;
    using Reader = ::io::MemoryQueueReader<Queue>;
    using Writer = ::io::MemoryQueueWriter<Queue>;

    PduTransportTxAdapterTest()
    : txQueue()
    , txReader(txQueue)
    , txWriter(txQueue)
    , memAllocationFailureCounter(0)
    , errorHandler(
          ::routing::ErrorHandler::Function::create<
              PduTransportTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE>,
              &PduTransportTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE>::handleError>(*this),
          0U)
    , txAdapter(txWriter, errorHandler)
    {}

    void handleError(::routing::ErrorHandler::StatusCode const statusCode, uint8_t, uint32_t)
    {
        switch (statusCode)
        {
            case ::routing::ErrorHandler::StatusCode::MEM_ALLOCATION_FAILURE:
                ++memAllocationFailureCounter;
                break;
            default: break;
        }
    }

    Queue txQueue;
    Reader txReader;
    Writer txWriter;

    uint32_t memAllocationFailureCounter;

    ::routing::ErrorHandler errorHandler;

    ::routing::PduTransportTxAdapter txAdapter;
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
TEST(PduTransportTxAdapterTest, max_size)
{
    constexpr size_t CAPACITY         = 128;
    constexpr size_t MAX_ELEMENT_SIZE = 16;

    PduTransportTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest;
    auto& txAdapter = txAdapterTest.txAdapter;
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.maxSize());
}

/**
 * \desc
 * Commiting without allocation does nothing.
 */
TEST(PduTransportTxAdapterTest, commit_without_allocation)
{
    constexpr size_t CAPACITY         = 128;
    constexpr size_t MAX_ELEMENT_SIZE = 16;

    PduTransportTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest;
    auto& txReader  = txAdapterTest.txReader;
    auto& txAdapter = txAdapterTest.txAdapter;

    txAdapter.commit();

    EXPECT_EQ(0, txReader.peek().size());
    EXPECT_EQ(0, txAdapter.pduCounter());
    EXPECT_EQ(0, txAdapter.byteCounter());
    EXPECT_EQ(0, txAdapter.droppedPduCounter());
    EXPECT_EQ(0, txAdapter.droppedByteCounter());
}

/**
 * \desc
 * Sending a PDU through the PduTransportTxAdapter doesn't modify it.
 */
TEST(PduTransportTxAdapterTest, allocate_commit)
{
    constexpr size_t SIZE = 16;
    PduTransportTxAdapterTest<> txAdapterTest;
    ::etl::vector<uint8_t, SIZE> sentPdu(SIZE);

    auto& txReader  = txAdapterTest.txReader;
    auto& txAdapter = txAdapterTest.txAdapter;

    ::etl::copy(allocatePdu(0xAA, SIZE, txAdapter), ::etl::make_span(sentPdu));
    txAdapter.commit();

    EXPECT_EQ(SIZE, txReader.peek().size());
    EXPECT_THAT(txReader.peek(), ElementsAreArray(sentPdu));
    EXPECT_EQ(1, txAdapter.pduCounter());
    EXPECT_EQ(SIZE, txAdapter.byteCounter());
    EXPECT_EQ(0, txAdapter.droppedPduCounter());
    EXPECT_EQ(0, txAdapter.droppedByteCounter());
}

/**
 * \desc
 * PDU is dropped on allocation if queue is full.
 */
TEST(PduTransportTxAdapterTest, drop_pdu_on_allocation_queue_is_full)
{
    constexpr size_t MAX_ELEMENT_SIZE = 16;
    constexpr size_t CAPACITY         = 32;

    PduTransportTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest;
    auto& txAdapter = txAdapterTest.txAdapter;

    writePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);

    auto pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);

    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(1, txAdapter.pduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.byteCounter());
    EXPECT_EQ(1, txAdapter.droppedPduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.droppedByteCounter());
    EXPECT_EQ(1, txAdapterTest.memAllocationFailureCounter);
    EXPECT_EQ(1, txAdapter.failedMemAllocPdus());
}

/**
 * \desc
 * PDU is dropped on allocation if message size is greater than max element size of queue.
 */
TEST(PduTransportTxAdapterTest, drop_pdu_on_allocation_message_is_too_big)
{
    constexpr size_t MAX_ELEMENT_SIZE = 16;
    constexpr size_t CAPACITY         = 32;

    PduTransportTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest;
    auto& txReader  = txAdapterTest.txReader;
    auto& txAdapter = txAdapterTest.txAdapter;

    auto pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);
    pdu      = allocatePdu(0x1, MAX_ELEMENT_SIZE + 1, txAdapter);
    txAdapter.commit();

    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(0, txReader.peek().size());
    EXPECT_EQ(0, txAdapter.pduCounter());
    EXPECT_EQ(0, txAdapter.byteCounter());
    EXPECT_EQ(1, txAdapter.droppedPduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE + 1, txAdapter.droppedByteCounter());
    EXPECT_EQ(1, txAdapterTest.memAllocationFailureCounter);
    EXPECT_EQ(1, txAdapter.failedMemAllocPdus());
}

/**
 * \desc Multiple consecutive drops only cause the error handling to be triggered once.
 */
TEST(PduTransportTxAdapterTest, multiple_drops_one_error_handling)
{
    constexpr size_t MAX_ELEMENT_SIZE = 16;
    constexpr size_t CAPACITY         = 32;

    PduTransportTxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> txAdapterTest;
    auto& txAdapter = txAdapterTest.txAdapter;

    writePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);

    auto pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);

    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(1, txAdapter.pduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.byteCounter());
    EXPECT_EQ(1, txAdapter.droppedPduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.droppedByteCounter());
    EXPECT_EQ(1, txAdapterTest.memAllocationFailureCounter);
    EXPECT_EQ(1, txAdapter.failedMemAllocPdus());

    pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);

    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(1, txAdapter.pduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE, txAdapter.byteCounter());
    EXPECT_EQ(2, txAdapter.droppedPduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE * 2, txAdapter.droppedByteCounter());
    EXPECT_EQ(1, txAdapterTest.memAllocationFailureCounter); // only one call
    EXPECT_EQ(2, txAdapter.failedMemAllocPdus());

    // Succesfully allocating a PDU then failing counts again
    auto reader = txAdapterTest.txReader;
    reader.peek();
    reader.release();
    writePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);
    pdu = allocatePdu(0x1, MAX_ELEMENT_SIZE, txAdapter);

    EXPECT_EQ(0, pdu.size());
    EXPECT_EQ(2, txAdapter.pduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE * 2, txAdapter.byteCounter());
    EXPECT_EQ(3, txAdapter.droppedPduCounter());
    EXPECT_EQ(MAX_ELEMENT_SIZE * 3, txAdapter.droppedByteCounter());
    EXPECT_EQ(2, txAdapterTest.memAllocationFailureCounter); // this one is counted
    EXPECT_EQ(3, txAdapter.failedMemAllocPdus());
}

/**
 * \desc
 *  Flush triggers the output writer's flush method.
 */
TEST(PduTransportTxAdapterTest, flush)
{
    ::io::IWriterMock writerMock;

    EXPECT_CALL(writerMock, flush()).Times(1);

    ::routing::PduTransportTxAdapter txAdapter(writerMock, {});

    txAdapter.flush();
}
} // namespace
