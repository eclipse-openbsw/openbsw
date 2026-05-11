/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/RxAdapter.h"

#include "blob/configuration.h"
#include "routing/ErrorHandler.h"
#include "routing/definition.h"
#include "routing/util.h"

#include <blob/Blob.h>
#include <etl/array.h>
#include <etl/optional.h>
#include <io/MemoryQueue.h>

#include <gmock/gmock.h>

#include <vector>

namespace
{
using namespace ::testing;

template<size_t CAPACITY = 1024, size_t MAX_ELEMENT_SIZE = 128>
struct RxAdapterTest
{
    using Queue  = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE>;
    using Reader = ::io::MemoryQueueReader<Queue>;
    using Writer = ::io::MemoryQueueWriter<Queue>;

    RxAdapterTest(::etl::span<::routing::Definition> defs)
    : rxQueue()
    , rxReader(rxQueue)
    , rxWriter(rxQueue)
    , invalidHeaderSizeCounter(0)
    , unknownMessageIdCounter(0)
    , invalidParsedLengthCounter(0)
    , invalidPayloadLengthCounter(0)
    , errorHandler(
          ::routing::ErrorHandler::Function::create<
              RxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE>,
              &RxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE>::handleError>(*this),
          0U)
    {
        routing::load(rxAdapterTable, defs, 0, rxAdapterTableMem);
        (void)rxAdapter.emplace(rxReader, buffer, rxAdapterTable, errorHandler);
    }

    void handleError(::routing::ErrorHandler::StatusCode const statusCode, uint8_t, uint32_t)
    {
        switch (statusCode)
        {
            case ::routing::ErrorHandler::StatusCode::INVALID_MESSAGE_HEADER_SIZE:
                ++invalidHeaderSizeCounter;
                break;
            case ::routing::ErrorHandler::StatusCode::UNKNOWN_MESSAGE_ID:
                ++unknownMessageIdCounter;
                break;
            case ::routing::ErrorHandler::StatusCode::INVALID_PARSED_LENGTH:
                ++invalidParsedLengthCounter;
                break;
            case ::routing::ErrorHandler::StatusCode::INVALID_PAYLOAD_LENGTH:
                ++invalidPayloadLengthCounter;
                break;
            default: break;
        }
    }

    Queue rxQueue;
    Reader rxReader;
    Writer rxWriter;

    ::etl::array<uint8_t, MAX_ELEMENT_SIZE> buffer;

    ::routing::RxAdapterTableMem rxAdapterTableMem;
    ::routing::RxAdapterTable rxAdapterTable;

    ::etl::optional<::routing::RxAdapter> rxAdapter;

    uint32_t invalidHeaderSizeCounter;
    uint32_t unknownMessageIdCounter;
    uint32_t invalidParsedLengthCounter;
    uint32_t invalidPayloadLengthCounter;

    ::routing::ErrorHandler errorHandler;
};

::etl::span<uint8_t> allocatePdu(uint32_t const id, size_t const size, ::io::IWriter& writer)
{
    auto pdu = writer.allocate(size);
    if (pdu.size() >= ::routing::RxAdapter::MESSAGE_HEADER_SIZE)
    {
        auto header                       = pdu.first(::routing::RxAdapter::MESSAGE_HEADER_SIZE);
        header.take<::etl::be_uint32_t>() = id;
        auto& length                      = header.take<::etl::be_uint32_t>();
        length                            = size - ::routing::RxAdapter::MESSAGE_HEADER_SIZE;
    }
    return pdu;
}

void writePdu(uint32_t const id, size_t const size, ::io::IWriter& writer)
{
    auto pdu = allocatePdu(id, size, writer);
    if (!pdu.empty())
    {
        writer.commit();
    }
}

/**
 * \desc
 * The maxSize function returns the size of the buffer provided.
 */
TEST(RxAdapterTest, max_size)
{
    RxAdapterTest<> rxAdapterTest({});
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    EXPECT_EQ(rxAdapterTest.buffer.size(), rxAdapter.maxSize());
}

/**
 * \desc
 * A single PDU is extracted out of a message.
 */
TEST(RxAdapterTest, extract_pdu)
{
    constexpr uint32_t MESSAGE_ID           = 1;
    constexpr uint32_t OFFSET               = 0;
    constexpr uint32_t PAYLOAD_LENGTH       = 8;
    constexpr uint32_t PDU_ID               = 0;
    uint8_t expectedPayload[PAYLOAD_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7};

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, MESSAGE_ID, OFFSET, PAYLOAD_LENGTH)};
    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    auto message    = allocatePdu(MESSAGE_ID, size, rxWriter);
    ::etl::copy(
        ::etl::make_span(expectedPayload),
        message.subspan(::routing::RxAdapter::MESSAGE_HEADER_SIZE));
    rxWriter.commit();

    auto const pdu = rxAdapter.peek();
    EXPECT_EQ(size, pdu.size());
    EXPECT_EQ(1, rxAdapter.pduCounter());
    EXPECT_EQ(size, rxAdapter.byteCounter());
    EXPECT_EQ(0, rxAdapter.discardedPduCounter());
    EXPECT_EQ(0, rxAdapter.discardedByteCounter());

    auto res    = pdu;
    uint32_t id = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(PDU_ID, id);

    uint32_t l = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(PAYLOAD_LENGTH, l);
    EXPECT_THAT(res, ElementsAreArray(expectedPayload));
    EXPECT_THAT(rxAdapter.peek(), ElementsAreArray(pdu));

    rxAdapter.release();
    EXPECT_EQ(0, rxAdapter.peek().size());
}

/**
 * \desc
 * Multiple PDUs are extracted out of a single message.
 */
TEST(RxAdapterTest, extract_multiple_pdus)
{
    constexpr uint32_t MESSAGE_ID           = 1;
    constexpr uint8_t PAYLOAD_LENGTH        = 8;
    uint8_t expectedPayload[PAYLOAD_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7};

    constexpr size_t NUMBER_OF_PDUS         = 3;
    uint32_t offsets[NUMBER_OF_PDUS]        = {0, 0, 4};
    uint32_t payloadLengths[NUMBER_OF_PDUS] = {8, 4, 4};

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, MESSAGE_ID, offsets[0], payloadLengths[0]),
           ::routing::Definition().in(0, MESSAGE_ID, offsets[1], payloadLengths[1]),
           ::routing::Definition().in(0, MESSAGE_ID, offsets[2], payloadLengths[2])};
    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    auto message    = allocatePdu(MESSAGE_ID, size, rxWriter);
    ::etl::copy(
        ::etl::make_span(expectedPayload),
        message.subspan(::routing::RxAdapter::MESSAGE_HEADER_SIZE));
    rxWriter.commit();

    for (size_t i = 0; i < NUMBER_OF_PDUS; ++i)
    {
        auto const pdu = rxAdapter.peek();
        EXPECT_EQ(::routing::RxAdapter::MESSAGE_HEADER_SIZE + payloadLengths[i], pdu.size());
        EXPECT_EQ(1, rxAdapter.pduCounter());
        EXPECT_EQ(size, rxAdapter.byteCounter());
        EXPECT_EQ(0, rxAdapter.discardedPduCounter());
        EXPECT_EQ(0, rxAdapter.discardedByteCounter());

        auto res    = pdu;
        uint32_t id = res.take<::etl::be_uint32_t>();
        EXPECT_EQ(i, id);

        uint32_t l = res.take<::etl::be_uint32_t>();
        EXPECT_EQ(payloadLengths[i], l);
        EXPECT_THAT(
            res,
            ElementsAreArray(
                ::etl::make_span(expectedPayload).subspan(offsets[i], payloadLengths[i])));
        EXPECT_THAT(rxAdapter.peek(), ElementsAreArray(pdu));

        rxAdapter.release();
    }

    EXPECT_EQ(0, rxAdapter.peek().size());
}

/**
 * \desc
 * Instead of extracting two PDUs, only one is extracted out of a shorter message.
 */
TEST(RxAdapterTest, extract_pdu_from_shorter_message)
{
    constexpr uint32_t MESSAGE_ID           = 1;
    constexpr uint32_t OFFSET               = 0;
    constexpr uint32_t PAYLOAD_LENGTH       = 4;
    constexpr uint32_t PDU_ID               = 0;
    uint8_t expectedPayload[PAYLOAD_LENGTH] = {0, 1, 2, 3};

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, MESSAGE_ID, OFFSET, PAYLOAD_LENGTH),
           ::routing::Definition().in(0, MESSAGE_ID, OFFSET, PAYLOAD_LENGTH + 1)};

    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    auto message    = allocatePdu(MESSAGE_ID, size, rxWriter);
    ::etl::copy(
        ::etl::make_span(expectedPayload),
        message.subspan(::routing::RxAdapter::MESSAGE_HEADER_SIZE));
    rxWriter.commit();

    auto const pdu = rxAdapter.peek();
    EXPECT_EQ(::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH, pdu.size());
    EXPECT_EQ(1, rxAdapter.pduCounter());
    EXPECT_EQ(size, rxAdapter.byteCounter());
    EXPECT_EQ(0, rxAdapter.discardedPduCounter());
    EXPECT_EQ(0, rxAdapter.discardedByteCounter());

    auto res    = pdu;
    uint32_t id = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(PDU_ID, id);

    uint32_t l = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(PAYLOAD_LENGTH, l);
    EXPECT_THAT(res, ElementsAreArray(expectedPayload));
    EXPECT_THAT(rxAdapter.peek(), ElementsAreArray(pdu));

    rxAdapter.release();
    EXPECT_EQ(0, rxAdapter.peek().size());
}

/**
 * \desc
 * A PDU is extracted out of a longer message.
 */
TEST(RxAdapterTest, extract_pdu_from_longer_message)
{
    constexpr uint32_t MESSAGE_ID           = 1;
    constexpr uint32_t OFFSET               = 0;
    constexpr uint32_t PAYLOAD_LENGTH       = 8;
    constexpr uint32_t PDU_ID               = 0;
    uint8_t expectedPayload[PAYLOAD_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7};

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, MESSAGE_ID, OFFSET, PAYLOAD_LENGTH)};
    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH + 1;
    auto message    = allocatePdu(MESSAGE_ID, size, rxWriter);
    ::etl::copy(
        ::etl::make_span(expectedPayload),
        message.subspan(::routing::RxAdapter::MESSAGE_HEADER_SIZE));
    rxWriter.commit();

    auto const pdu = rxAdapter.peek();
    EXPECT_EQ(::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH, pdu.size());
    EXPECT_EQ(1, rxAdapter.pduCounter());
    EXPECT_EQ(size, rxAdapter.byteCounter());
    EXPECT_EQ(0, rxAdapter.discardedPduCounter());
    EXPECT_EQ(0, rxAdapter.discardedByteCounter());

    auto res    = pdu;
    uint32_t id = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(PDU_ID, id);

    uint32_t l = res.take<::etl::be_uint32_t>();
    EXPECT_EQ(PAYLOAD_LENGTH, l);
    EXPECT_THAT(res, ElementsAreArray(expectedPayload));
    EXPECT_THAT(rxAdapter.peek(), ElementsAreArray(pdu));

    rxAdapter.release();
    EXPECT_EQ(0, rxAdapter.peek().size());
}

/**
 * \desc
 * Multiple PDUs are extracted out of some messages.
 */
TEST(RxAdapterTest, extract_pdus_from_some_messages)
{
    constexpr size_t NUMBER_OF_PDUS     = 6;
    constexpr size_t MAX_PAYLOAD_LENGTH = 8;

    uint32_t messageIds[NUMBER_OF_PDUS]         = {1, 2, 2, 3, 3, 3};
    uint32_t offsets[NUMBER_OF_PDUS]            = {0, 0, 4, 0, 4, 0};
    uint32_t payloadLengths[NUMBER_OF_PDUS]     = {8, 8, 4, 8, 4, 4};
    uint8_t expectedPayload[MAX_PAYLOAD_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7};
    uint32_t expectedPduCounter = 0U, expectedByteCounter = 0U;

    ::etl::vector<::routing::Definition, NUMBER_OF_PDUS> defs;
    for (size_t i = 0; i < NUMBER_OF_PDUS; ++i)
    {
        defs.emplace_back(
            ::routing::Definition().in(0, messageIds[i], offsets[i], payloadLengths[i]));
    }

    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size    = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + MAX_PAYLOAD_LENGTH;
    uint32_t messageId = 0;
    for (size_t i = 0; i < NUMBER_OF_PDUS; ++i)
    {
        if (messageId != messageIds[i])
        {
            messageId    = messageIds[i];
            auto message = allocatePdu(messageId, size, rxWriter);
            ::etl::copy(
                ::etl::make_span(expectedPayload),
                message.subspan(::routing::RxAdapter::MESSAGE_HEADER_SIZE));
            rxWriter.commit();

            ++expectedPduCounter;
            expectedByteCounter += size;
        }

        auto const payloadLength = payloadLengths[i];
        auto const offset        = offsets[i];

        auto const pdu = rxAdapter.peek();
        EXPECT_EQ(pdu.size(), ::routing::RxAdapter::MESSAGE_HEADER_SIZE + payloadLength);
        EXPECT_EQ(expectedPduCounter, rxAdapter.pduCounter());
        EXPECT_EQ(expectedByteCounter, rxAdapter.byteCounter());
        EXPECT_EQ(rxAdapter.discardedPduCounter(), 0);
        EXPECT_EQ(rxAdapter.discardedByteCounter(), 0);

        auto res    = pdu;
        uint32_t id = res.take<::etl::be_uint32_t>();
        EXPECT_EQ(i, id);

        uint32_t l = res.take<::etl::be_uint32_t>();
        EXPECT_EQ(payloadLength, l);

        EXPECT_THAT(
            res,
            ElementsAreArray(::etl::make_span(expectedPayload).subspan(offset, payloadLength)));
        EXPECT_THAT(rxAdapter.peek(), ElementsAreArray(pdu));

        rxAdapter.release();
    }

    EXPECT_EQ(0, rxAdapter.peek().size());
}

/**
 * \desc
 * PDU is dropped if message header size is invalid.
 */
TEST(RxAdapterTest, discard_pdu_invalid_message_header_size)
{
    constexpr uint32_t MESSAGE_ID = 1;

    ::routing::Definition defs[] = {::routing::Definition().in(0, MESSAGE_ID, 0, 8)};
    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE - 1;
    writePdu(MESSAGE_ID, size, rxWriter);

    EXPECT_EQ(0, rxAdapter.peek().size());
    EXPECT_EQ(1, rxAdapter.pduCounter());
    EXPECT_EQ(size, rxAdapter.byteCounter());
    EXPECT_EQ(1, rxAdapter.discardedPduCounter());
    EXPECT_EQ(size, rxAdapter.discardedByteCounter());
    EXPECT_EQ(1, rxAdapterTest.invalidHeaderSizeCounter);
    EXPECT_EQ(1, rxAdapter.invalidHeaderSizePdus());
    EXPECT_EQ(0, rxAdapterTest.unknownMessageIdCounter);
    EXPECT_EQ(0, rxAdapter.unknownMessagePdus());
    EXPECT_EQ(0, rxAdapterTest.invalidParsedLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidParsedPayloadLengthPdus());
    EXPECT_EQ(0, rxAdapterTest.invalidPayloadLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidPayloadLengthPdus());
}

/**
 * \desc
 * PDU is dropped if message ID is unknown.
 */
TEST(RxAdapterTest, discard_pdu_unknown_message_id)
{
    constexpr uint32_t MESSAGE_ID     = 1;
    constexpr uint32_t PAYLOAD_LENGTH = 8;

    ::routing::Definition defs[] = {::routing::Definition().in(0, MESSAGE_ID, 0, PAYLOAD_LENGTH)};
    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    writePdu(MESSAGE_ID + 1, size, rxWriter);

    EXPECT_EQ(0, rxAdapter.peek().size());
    EXPECT_EQ(1, rxAdapter.pduCounter());
    EXPECT_EQ(size, rxAdapter.byteCounter());
    EXPECT_EQ(1, rxAdapter.discardedPduCounter());
    EXPECT_EQ(size, rxAdapter.discardedByteCounter());
    EXPECT_EQ(0, rxAdapterTest.invalidHeaderSizeCounter);
    EXPECT_EQ(0, rxAdapter.invalidHeaderSizePdus());
    EXPECT_EQ(1, rxAdapterTest.unknownMessageIdCounter);
    EXPECT_EQ(1, rxAdapter.unknownMessagePdus());
    EXPECT_EQ(0, rxAdapterTest.invalidParsedLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidParsedPayloadLengthPdus());
    EXPECT_EQ(0, rxAdapterTest.invalidPayloadLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidPayloadLengthPdus());
}

/**
 * \desc
 * PDU is dropped if parsed length is invalid.
 */
TEST(RxAdapterTest, discard_pdu_invalid_parsed_length)
{
    constexpr uint32_t MESSAGE_ID     = 1;
    constexpr uint32_t PAYLOAD_LENGTH = 8;

    ::routing::Definition defs[] = {::routing::Definition().in(0, MESSAGE_ID, 0, PAYLOAD_LENGTH)};
    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size                = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    auto pdu                       = rxWriter.allocate(size);
    pdu.take<::etl::be_uint32_t>() = MESSAGE_ID;
    pdu.take<::etl::be_uint32_t>() = PAYLOAD_LENGTH + 1;
    rxWriter.commit();

    EXPECT_EQ(0, rxAdapter.peek().size());
    EXPECT_EQ(1, rxAdapter.pduCounter());
    EXPECT_EQ(size, rxAdapter.byteCounter());
    EXPECT_EQ(1, rxAdapter.discardedPduCounter());
    EXPECT_EQ(size, rxAdapter.discardedByteCounter());
    EXPECT_EQ(0, rxAdapterTest.invalidHeaderSizeCounter);
    EXPECT_EQ(0, rxAdapter.invalidHeaderSizePdus());
    EXPECT_EQ(0, rxAdapterTest.unknownMessageIdCounter);
    EXPECT_EQ(0, rxAdapter.unknownMessagePdus());
    EXPECT_EQ(1, rxAdapterTest.invalidParsedLengthCounter);
    EXPECT_EQ(1, rxAdapter.invalidParsedPayloadLengthPdus());
    EXPECT_EQ(0, rxAdapterTest.invalidPayloadLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidPayloadLengthPdus());
}

/**
 * \desc
 * PDU is discarded if payload length is invalid. If at least one PDU is extracted out of a message,
 * it is not discarded.
 */
TEST(RxAdapterTest, discard_pdu_invalid_payload_length)
{
    constexpr uint32_t MESSAGE_ID    = 1;
    constexpr uint8_t PAYLOAD_LENGTH = 4;

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, MESSAGE_ID, 0, PAYLOAD_LENGTH + 1),
           ::routing::Definition().in(0, MESSAGE_ID, 0, PAYLOAD_LENGTH)};
    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const validSize   = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    auto const invalidSize = validSize - 1;

    writePdu(MESSAGE_ID, validSize, rxWriter);

    EXPECT_EQ(validSize, rxAdapter.peek().size());
    EXPECT_EQ(1, rxAdapter.pduCounter());
    EXPECT_EQ(validSize, rxAdapter.byteCounter());
    EXPECT_EQ(0, rxAdapter.discardedPduCounter());
    EXPECT_EQ(0, rxAdapter.discardedByteCounter());
    EXPECT_EQ(0, rxAdapterTest.invalidHeaderSizeCounter);
    EXPECT_EQ(0, rxAdapter.invalidHeaderSizePdus());
    EXPECT_EQ(0, rxAdapterTest.unknownMessageIdCounter);
    EXPECT_EQ(0, rxAdapter.unknownMessagePdus());
    EXPECT_EQ(0, rxAdapterTest.invalidParsedLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidParsedPayloadLengthPdus());
    EXPECT_EQ(0, rxAdapterTest.invalidPayloadLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidPayloadLengthPdus());

    rxAdapter.release();
    writePdu(MESSAGE_ID, invalidSize, rxWriter);

    EXPECT_EQ(0, rxAdapter.peek().size());
    EXPECT_EQ(2, rxAdapter.pduCounter());
    EXPECT_EQ(validSize + invalidSize, rxAdapter.byteCounter());
    EXPECT_EQ(1, rxAdapter.discardedPduCounter());
    EXPECT_EQ(invalidSize, rxAdapter.discardedByteCounter());
    EXPECT_EQ(0, rxAdapterTest.invalidHeaderSizeCounter);
    EXPECT_EQ(0, rxAdapter.invalidHeaderSizePdus());
    EXPECT_EQ(0, rxAdapterTest.unknownMessageIdCounter);
    EXPECT_EQ(0, rxAdapter.unknownMessagePdus());
    EXPECT_EQ(0, rxAdapterTest.invalidParsedLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidParsedPayloadLengthPdus());
    EXPECT_EQ(1, rxAdapterTest.invalidPayloadLengthCounter);
    EXPECT_EQ(1, rxAdapter.invalidPayloadLengthPdus());
}

/**
 * \desc
 * Invalid PDUs are discarded before extracting one from a message.
 */
TEST(RxAdapterTest, discard_invalid_pdus)
{
    constexpr uint32_t MESSAGE_ID    = 1;
    constexpr uint8_t PAYLOAD_LENGTH = 8;

    ::routing::Definition defs[] = {::routing::Definition().in(0, MESSAGE_ID, 0, PAYLOAD_LENGTH)};
    RxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter             = *rxAdapterTest.rxAdapter;
    auto& rxWriter              = rxAdapterTest.rxWriter;
    uint32_t expectedPduCounter = 0U, expectedByteCounter = 0U, expectedDiscardedPduCounter = 0U,
             expectedDiscardedByteCounter = 0U;

    constexpr size_t size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    // invalid header size
    writePdu(MESSAGE_ID, ::routing::RxAdapter::MESSAGE_HEADER_SIZE - 1, rxWriter);
    // unknown message id
    writePdu(MESSAGE_ID + 1, size, rxWriter);
    // invalid parsed length
    auto message                       = rxWriter.allocate(size);
    message.take<::etl::be_uint32_t>() = MESSAGE_ID;
    message.take<::etl::be_uint32_t>() = PAYLOAD_LENGTH + 1;
    rxWriter.commit();
    // invalid payload length
    writePdu(MESSAGE_ID, size - 1, rxWriter);

    writePdu(MESSAGE_ID, ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH, rxWriter);

    auto pdu = rxAdapter.peek();

    expectedPduCounter += 5;
    expectedByteCounter += 70;
    expectedDiscardedPduCounter += 4;
    expectedDiscardedByteCounter += 54;

    EXPECT_EQ(::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH, pdu.size());
    EXPECT_EQ(expectedPduCounter, rxAdapter.pduCounter());
    EXPECT_EQ(expectedByteCounter, rxAdapter.byteCounter());
    EXPECT_EQ(expectedDiscardedPduCounter, rxAdapter.discardedPduCounter());
    EXPECT_EQ(expectedDiscardedByteCounter, rxAdapter.discardedByteCounter());
}

} // namespace
