/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/PduTransportRxAdapter.h"

#include "routing/ErrorHandler.h"
#include "routing/definition.h"

#include <etl/optional.h>
#include <etl/span.h>
#include <io/MemoryQueue.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

template<size_t CAPACITY = 1024, size_t MAX_ELEMENT_SIZE = 128>
struct PduTransportRxAdapterTest
{
    using Queue     = ::io::MemoryQueue<CAPACITY, MAX_ELEMENT_SIZE>;
    using Reader    = ::io::MemoryQueueReader<Queue>;
    using Writer    = ::io::MemoryQueueWriter<Queue>;
    using RxAdapter = ::routing::PduTransportRxAdapter<MAX_ELEMENT_SIZE>;

    PduTransportRxAdapterTest(::etl::span<::routing::Definition> defs)
    : rxQueue()
    , rxReader(rxQueue)
    , rxWriter(rxQueue)
    , invalidHeaderSizeCounter(0)
    , unknownMessageIdCounter(0)
    , invalidParsedLengthCounter(0)
    , invalidMessageLengthCounter(0)
    , invalidPayloadLengthCounter(0)
    , errorHandler(
          ::routing::ErrorHandler::Function::create<
              PduTransportRxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE>,
              &PduTransportRxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE>::handleError>(*this),
          0U)
    {
        routing::load(rxAdapterTable, defs, 0, rxAdapterTableMem);
        (void)rxAdapter.emplace(rxReader, rxAdapterTable, errorHandler);
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
            case ::routing::ErrorHandler::StatusCode::INVALID_MESSAGE_LENGTH:
                ++invalidMessageLengthCounter;
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

    ::routing::RxAdapterTableMem rxAdapterTableMem;
    ::routing::RxAdapterTable rxAdapterTable;

    ::etl::optional<RxAdapter> rxAdapter;

    uint32_t invalidHeaderSizeCounter;
    uint32_t unknownMessageIdCounter;
    uint32_t invalidParsedLengthCounter;
    uint32_t invalidMessageLengthCounter;
    uint32_t invalidPayloadLengthCounter;

    ::routing::ErrorHandler errorHandler;
};

::etl::span<uint8_t> writePdu(::etl::span<uint8_t>& frame, uint32_t const id, size_t const size)
{
    auto const pdu = frame.size() > size ? frame.first(size) : frame;
    if (pdu.size() >= ::routing::RxAdapter::MESSAGE_HEADER_SIZE)
    {
        auto header                       = pdu.first(::routing::RxAdapter::MESSAGE_HEADER_SIZE);
        header.take<::etl::be_uint32_t>() = id;
        auto& length                      = header.take<::etl::be_uint32_t>();
        length                            = size - ::routing::RxAdapter::MESSAGE_HEADER_SIZE;
    }
    frame.advance(size);
    return pdu;
}

::etl::span<uint8_t> allocateFrame(uint32_t const id, size_t const size, ::io::IWriter& writer)
{
    auto frame = writer.allocate(size);
    if (!frame.empty())
    {
        ::etl::span<uint8_t> pdu = frame;
        writePdu(pdu, id, size);
    }
    return frame;
}

void writeFrame(uint32_t const id, size_t const size, ::io::IWriter& writer)
{
    auto frame = allocateFrame(id, size, writer);
    if (!frame.empty())
    {
        writer.commit();
    }
}

/**
 * \desc
 * The max_size function returns the correct value.
 */
TEST(PduTransportRxAdapterTest, max_size)
{
    constexpr size_t CAPACITY         = 128;
    constexpr size_t MAX_ELEMENT_SIZE = 16;

    PduTransportRxAdapterTest<CAPACITY, MAX_ELEMENT_SIZE> rxAdapterTest({});
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    EXPECT_EQ(rxAdapter.maxSize(), MAX_ELEMENT_SIZE);
}

/**
 * \desc
 * A single PDU is extracted out of a single frame with one message.
 */
TEST(PduTransportRxAdapterTest, extract_pdu_from_one_frame_with_one_message)
{
    constexpr uint32_t MESSAGE_ID           = 1;
    constexpr uint32_t OFFSET               = 0;
    constexpr uint32_t PAYLOAD_LENGTH       = 8;
    constexpr uint32_t PDU_ID               = 0;
    uint8_t expectedPayload[PAYLOAD_LENGTH] = {0, 1, 2, 3, 4, 5, 6, 7};

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, MESSAGE_ID, OFFSET, PAYLOAD_LENGTH)};
    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;

    auto message = allocateFrame(MESSAGE_ID, size, rxWriter);
    EXPECT_TRUE(::etl::copy(
        ::etl::make_span(expectedPayload),
        message.subspan(::routing::RxAdapter::MESSAGE_HEADER_SIZE)));
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
 * Multiple PDUs are extracted out of a single frame with one message.
 */
TEST(PduTransportRxAdapterTest, extract_multiple_pdus_from_one_frame_with_one_message)
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
    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    auto message    = allocateFrame(MESSAGE_ID, size, rxWriter);
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
 * Multiple PDUs are extracted out of a single frame with multiple messages.
 */
TEST(PduTransportRxAdapterTest, extract_multiple_pdus_from_one_frame_with_multiple_messages)
{
    constexpr size_t NUMBER_OF_MESSAGES = 3;
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

    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + MAX_PAYLOAD_LENGTH;

    uint32_t messageId = 0;
    auto frame         = rxWriter.allocate(NUMBER_OF_MESSAGES * size);
    for (size_t i = 0; i < NUMBER_OF_PDUS; ++i)
    {
        if (messageId != messageIds[i])
        {
            messageId    = messageIds[i];
            auto message = writePdu(frame, messageId, size);
            ::etl::copy(
                ::etl::make_span(expectedPayload),
                message.subspan(::routing::RxAdapter::MESSAGE_HEADER_SIZE));
        }
    }
    rxWriter.commit();

    messageId = 0;
    for (size_t i = 0; i < NUMBER_OF_PDUS; ++i)
    {
        if (messageId != messageIds[i])
        {
            messageId = messageIds[i];
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
 * Multiple PDUs are extracted out of multiple frames with one message each.
 */
TEST(PduTransportRxAdapterTest, extract_multiple_pdus_from_multiple_frames_with_one_message_each)
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

    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size    = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + MAX_PAYLOAD_LENGTH;
    uint32_t messageId = 0;
    for (size_t i = 0; i < NUMBER_OF_PDUS; ++i)
    {
        if (messageId != messageIds[i])
        {
            messageId    = messageIds[i];
            auto frame   = rxWriter.allocate(size);
            auto message = writePdu(frame, messageId, size);
            ::etl::copy(
                ::etl::make_span(expectedPayload),
                message.subspan(::routing::RxAdapter::MESSAGE_HEADER_SIZE));
            rxWriter.commit();
        }
    }

    messageId = 0;
    for (size_t i = 0; i < NUMBER_OF_PDUS; ++i)
    {
        if (messageId != messageIds[i])
        {
            messageId = messageIds[i];
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
 * Multiple PDUs are extracted out of multiple frames with multiple messages each.
 */
TEST(
    PduTransportRxAdapterTest,
    extract_multiple_pdus_multiple_from_frames_with_multiple_messages_each)
{
    constexpr size_t NUMBER_OF_FRAMES   = 3;
    constexpr size_t NUMBER_OF_MESSAGES = 3;
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

    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + MAX_PAYLOAD_LENGTH;

    uint32_t messageId = 0;
    auto messages      = rxWriter.allocate(NUMBER_OF_MESSAGES * size);
    auto frame         = messages;
    for (size_t i = 0; i < NUMBER_OF_PDUS; ++i)
    {
        if (messageId != messageIds[i])
        {
            messageId    = messageIds[i];
            auto message = writePdu(frame, messageId, size);
            ::etl::copy(
                ::etl::make_span(expectedPayload),
                message.subspan(::routing::RxAdapter::MESSAGE_HEADER_SIZE));
        }
    }

    for (size_t i = 0; i < NUMBER_OF_FRAMES; ++i)
    {
        auto frame = rxWriter.allocate(messages.size());
        ::etl::copy(messages, frame);
        rxWriter.commit();
    }

    for (size_t j = 0; j < NUMBER_OF_FRAMES; ++j)
    {
        messageId = 0;
        for (size_t i = 0; i < NUMBER_OF_PDUS; ++i)
        {
            if (messageId != messageIds[i])
            {
                messageId = messageIds[i];
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
    }
    EXPECT_EQ(0, rxAdapter.peek().size());
}

/**
 * \desc
 * PDU is dropped if message header size is invalid.
 */
TEST(PduTransportRxAdapterTest, discard_pdu_invalid_message_header_size)
{
    constexpr uint32_t MESSAGE_ID = 1;

    ::routing::Definition defs[] = {::routing::Definition().in(0, MESSAGE_ID, 0, 8)};
    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE - 1;
    writeFrame(MESSAGE_ID, size, rxWriter);

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
TEST(PduTransportRxAdapterTest, discard_pdu_unknown_message_id)
{
    constexpr size_t NUMBER_OF_PDUS   = 3;
    constexpr uint32_t MESSAGE_ID     = 1;
    constexpr uint32_t PAYLOAD_LENGTH = 8;

    ::routing::Definition defs[] = {::routing::Definition().in(0, MESSAGE_ID, 0, PAYLOAD_LENGTH)};
    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;

    writeFrame(MESSAGE_ID + 1, size, rxWriter);
    auto frame = rxWriter.allocate(2 * size);
    writePdu(frame, MESSAGE_ID + 1, size);
    writePdu(frame, MESSAGE_ID + 1, size);
    rxWriter.commit();

    EXPECT_EQ(0, rxAdapter.peek().size());
    EXPECT_EQ(NUMBER_OF_PDUS, rxAdapter.pduCounter());
    EXPECT_EQ(NUMBER_OF_PDUS * size, rxAdapter.byteCounter());
    EXPECT_EQ(NUMBER_OF_PDUS, rxAdapter.discardedPduCounter());
    EXPECT_EQ(NUMBER_OF_PDUS * size, rxAdapter.discardedByteCounter());
    EXPECT_EQ(0, rxAdapterTest.invalidHeaderSizeCounter);
    EXPECT_EQ(0, rxAdapter.invalidHeaderSizePdus());
    EXPECT_EQ(NUMBER_OF_PDUS, rxAdapterTest.unknownMessageIdCounter);
    EXPECT_EQ(NUMBER_OF_PDUS, rxAdapter.unknownMessagePdus());
    EXPECT_EQ(0, rxAdapterTest.invalidParsedLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidParsedPayloadLengthPdus());
    EXPECT_EQ(0, rxAdapterTest.invalidPayloadLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidPayloadLengthPdus());
}

/**
 * \desc
 * PDU is dropped if parsed length is invalid.
 */
TEST(PduTransportRxAdapterTest, discard_pdu_invalid_parsed_length)
{
    constexpr uint32_t MESSAGE_ID     = 1;
    constexpr uint32_t PAYLOAD_LENGTH = 8;

    ::routing::Definition defs[] = {::routing::Definition().in(0, MESSAGE_ID, 0, PAYLOAD_LENGTH)};
    PduTransportRxAdapterTest<> rxAdapterTest(defs);
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
TEST(PduTransportRxAdapterTest, discard_pdu_invalid_payload_length)
{
    constexpr uint32_t MESSAGE_ID    = 1;
    constexpr uint8_t PAYLOAD_LENGTH = 4;

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, MESSAGE_ID, 0, PAYLOAD_LENGTH),
           ::routing::Definition().in(0, MESSAGE_ID, 0, PAYLOAD_LENGTH + 1)};
    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    auto const validSize   = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    auto const invalidSize = validSize - 1;

    writeFrame(MESSAGE_ID, validSize, rxWriter);
    writeFrame(MESSAGE_ID, invalidSize, rxWriter);
    auto frame = rxWriter.allocate(validSize + invalidSize);
    writePdu(frame, MESSAGE_ID, validSize);
    writePdu(frame, MESSAGE_ID, invalidSize);
    rxWriter.commit();

    EXPECT_EQ(validSize, rxAdapter.peek().size());
    EXPECT_EQ(1, rxAdapter.pduCounter());
    EXPECT_EQ(rxAdapter.byteCounter(), validSize);
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

    EXPECT_EQ(validSize, rxAdapter.peek().size());
    EXPECT_EQ(3, rxAdapter.pduCounter());
    EXPECT_EQ(2 * validSize + invalidSize, rxAdapter.byteCounter());
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

    rxAdapter.release();

    EXPECT_EQ(0, rxAdapter.peek().size());
    EXPECT_EQ(4, rxAdapter.pduCounter());
    EXPECT_EQ(2 * (validSize + invalidSize), rxAdapter.byteCounter());
    EXPECT_EQ(2, rxAdapter.discardedPduCounter());
    EXPECT_EQ(2 * invalidSize, rxAdapter.discardedByteCounter());
    EXPECT_EQ(0, rxAdapterTest.invalidHeaderSizeCounter);
    EXPECT_EQ(0, rxAdapter.invalidHeaderSizePdus());
    EXPECT_EQ(0, rxAdapterTest.unknownMessageIdCounter);
    EXPECT_EQ(0, rxAdapter.unknownMessagePdus());
    EXPECT_EQ(0, rxAdapterTest.invalidParsedLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidParsedPayloadLengthPdus());
    EXPECT_EQ(2, rxAdapterTest.invalidPayloadLengthCounter);
    EXPECT_EQ(2, rxAdapter.invalidPayloadLengthPdus());
}

/**
 * \desc
 * PDU is discarded if message length is invalid.
 */
TEST(PduTransportRxAdapterTest, discard_pdu_invalid_message_length)
{
    constexpr uint32_t NUMBER_OF_PDUS = 4;
    constexpr uint32_t MESSAGE_ID     = 1;
    constexpr uint8_t PAYLOAD_LENGTH  = 8;

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, MESSAGE_ID, PAYLOAD_LENGTH, 0, PAYLOAD_LENGTH),
           ::routing::Definition().in(0, MESSAGE_ID, PAYLOAD_LENGTH, 0, PAYLOAD_LENGTH - 1)};
    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter = *rxAdapterTest.rxAdapter;
    auto& rxWriter  = rxAdapterTest.rxWriter;

    constexpr size_t size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;

    writeFrame(MESSAGE_ID, size + 1, rxWriter);
    writeFrame(MESSAGE_ID, size - 1, rxWriter);
    auto frame = rxWriter.allocate(2 * size);
    writePdu(frame, MESSAGE_ID, size + 1);
    writePdu(frame, MESSAGE_ID, size - 1);
    rxWriter.commit();

    EXPECT_EQ(0, rxAdapter.peek().size());
    EXPECT_EQ(NUMBER_OF_PDUS, rxAdapter.pduCounter());
    EXPECT_EQ(NUMBER_OF_PDUS * size, rxAdapter.byteCounter());
    EXPECT_EQ(NUMBER_OF_PDUS, rxAdapter.discardedPduCounter());
    EXPECT_EQ(NUMBER_OF_PDUS * size, rxAdapter.discardedByteCounter());
    EXPECT_EQ(0, rxAdapterTest.invalidHeaderSizeCounter);
    EXPECT_EQ(0, rxAdapter.invalidHeaderSizePdus());
    EXPECT_EQ(0, rxAdapterTest.unknownMessageIdCounter);
    EXPECT_EQ(0, rxAdapter.unknownMessagePdus());
    EXPECT_EQ(0, rxAdapterTest.invalidParsedLengthCounter);
    EXPECT_EQ(0, rxAdapter.invalidParsedPayloadLengthPdus());
    EXPECT_EQ(NUMBER_OF_PDUS, rxAdapterTest.invalidMessageLengthCounter);
    EXPECT_EQ(NUMBER_OF_PDUS, rxAdapter.invalidMessageLengthPdus());
}

/**
 * \desc
 * Invalid PDUs are discarded before extracting one from a message.
 */
TEST(PduTransportRxAdapterTest, discard_invalid_pdus)
{
    constexpr uint32_t MESSAGE_ID    = 1;
    constexpr uint8_t PAYLOAD_LENGTH = 8;

    ::routing::Definition defs[]
        = {::routing::Definition().in(0, MESSAGE_ID, PAYLOAD_LENGTH, 0, PAYLOAD_LENGTH),
           ::routing::Definition().in(0, MESSAGE_ID + 1, 0, PAYLOAD_LENGTH)};
    PduTransportRxAdapterTest<> rxAdapterTest(defs);
    auto& rxAdapter             = *rxAdapterTest.rxAdapter;
    auto& rxWriter              = rxAdapterTest.rxWriter;
    uint32_t expectedPduCounter = 0U, expectedByteCounter = 0U, expectedDiscardedPduCounter = 0U,
             expectedDiscardedByteCounter = 0U;

    constexpr size_t size = ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH;
    // invalid header size
    writeFrame(MESSAGE_ID, ::routing::RxAdapter::MESSAGE_HEADER_SIZE - 1, rxWriter);
    // invalid parsed length
    auto frame                       = rxWriter.allocate(size);
    frame.take<::etl::be_uint32_t>() = MESSAGE_ID;
    frame.take<::etl::be_uint32_t>() = PAYLOAD_LENGTH + 1;
    rxWriter.commit();
    // unknown message id
    frame = rxWriter.allocate(4 * size);
    writePdu(frame, MESSAGE_ID + 2, size);
    // invalid message length
    writePdu(frame, MESSAGE_ID, size - 1);
    // invalid payload length
    writePdu(frame, MESSAGE_ID + 1, size - 1);

    writePdu(frame, MESSAGE_ID, ::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH);
    rxWriter.commit();

    auto pdu = rxAdapter.peek();

    expectedPduCounter += 6;
    expectedByteCounter += 85;
    expectedDiscardedPduCounter += 5;
    expectedDiscardedByteCounter += 69;

    EXPECT_EQ(::routing::RxAdapter::MESSAGE_HEADER_SIZE + PAYLOAD_LENGTH, pdu.size());
    EXPECT_EQ(expectedPduCounter, rxAdapter.pduCounter());
    EXPECT_EQ(expectedByteCounter, rxAdapter.byteCounter());
    EXPECT_EQ(expectedDiscardedPduCounter, rxAdapter.discardedPduCounter());
    EXPECT_EQ(expectedDiscardedByteCounter, rxAdapter.discardedByteCounter());
}

} // namespace
