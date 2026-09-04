/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TpReceiver.h"

#include "someip/ITpTransceiver.h"
#include "someip/NetworkChannel.h"
#include "someip/NetworkResourceMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"
#include "someip/TpListenerMock.h"

#include <etl/unaligned_type.h>
#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;

class TestTpReceiver : public TpReceiver
{
public:
    explicit TestTpReceiver(::etl::span<uint8_t> const& buffer) : TpReceiver(buffer) {}
};

struct TpReceiverTest : Test
{
    TpReceiverTest()
    {
        _resource.incRefCounter();
        EXPECT_CALL(_resource, getLocalPort()).Times(AnyNumber()).WillRepeatedly(Return(15U));
    }

    static void setupMessage(
        ::etl::span<uint8_t> buffer, size_t payloadOffset, size_t chunkLength, bool hasMoreSegments)
    {
        SomeIpMessage message(buffer);
        message.setMessageId(1U);
        message.setRequestId(2U);
        message.setProtocolVersion(3U);
        message.setInterfaceVersion(4U);
        message.setRawMessageType(
            static_cast<uint8_t>(SomeIpMessage::MessageType::REQUEST)
            | ITpTransceiver::TP_MESSAGE_TYPE_BIT_MASK);
        message.setReturnCode(static_cast<SomeIpMessage::ReturnCode>(5U));
        message.setPayloadLength(chunkLength + ITpTransceiver::TP_HEADER_LENGTH);

        ITpTransceiver::TpHeader tpHeader{};
        tpHeader.payloadOffset   = payloadOffset;
        tpHeader.hasMoreSegments = hasMoreSegments;
        etl::be_uint32_ext_t{&buffer[SomeIpMessage::OFFSET_PAYLOAD]}
        = ITpTransceiver::serializeTpHeader(tpHeader);

        for (size_t i = payloadOffset; i < (payloadOffset + chunkLength); ++i)
        {
            buffer.at(
                SomeIpMessage::OFFSET_PAYLOAD + i - payloadOffset
                + ITpTransceiver::TP_HEADER_LENGTH)
                = (i % 255U);
        }
    }

    static void verifyMessage(::etl::span<uint8_t> const& buffer, size_t payloadLength)
    {
        SomeIpMessage message(buffer);
        EXPECT_EQ(1U, message.getMessageId());
        EXPECT_EQ(2U, message.getRequestId());
        EXPECT_EQ(3U, message.getProtocolVersion());
        EXPECT_EQ(4U, message.getInterfaceVersion());
        EXPECT_EQ(
            SomeIpMessage::MessageType::REQUEST,
            static_cast<SomeIpMessage::MessageType>(message.getRawMessageType()));
        EXPECT_EQ(static_cast<SomeIpMessage::ReturnCode>(5U), message.getReturnCode());

        ::etl::span<uint8_t const> payload = message.getBufferPayload();

        for (size_t i = 0U; i < payloadLength; ++i)
        {
            EXPECT_EQ((i % 255U), payload.at(i));
        }
    }

    static size_t const TOTAL_MAX_TP_PAYLOAD_SIZE = TP_PAYLOAD_MAX_SIZE * 3.5;
    StrictMock<NetworkResourceMock> _resource;
    IPEndpoint _address{make_ip4(192U, 0U, 2U, 1U), 15U};
    NetworkChannel _channel{_resource, _address};

    ::etl::array<uint8_t, TOTAL_MAX_TP_PAYLOAD_SIZE + 20U /* tp-header */> _buffer{};
    SomeIpMessage _message{_buffer};

    StrictMock<TpListenerMock> _listener;
};

/**
 * Test TpReceiver lifecycle.
 */
TEST_F(TpReceiverTest, test_lifecycle)
{
    ::someip::declare::TpReceiver<TOTAL_MAX_TP_PAYLOAD_SIZE> receiver;
    EXPECT_FALSE(receiver.isActive());

    setupMessage(_buffer, 0U, 0U, false);

    EXPECT_FALSE(receiver.isMatching(_channel, _message));

    receiver.start(_channel, _message, _listener);

    EXPECT_TRUE(receiver.isActive());
    EXPECT_TRUE(receiver.isMatching(_channel, _message));

    receiver.stop();
    EXPECT_FALSE(receiver.isActive());
}

/**
 * Test successfully receiving a large message.
 */
TEST_F(TpReceiverTest, successfully_receive_large_message)
{
    size_t const payloadLength = TOTAL_MAX_TP_PAYLOAD_SIZE;

    ::etl::array<uint8_t, payloadLength + SomeIpConstants::HEADER_LENGTH> buffer{};
    TestTpReceiver receiver(buffer);

    TpReceiver::TpResult result;
    uint32_t timestamp = 1U;

    setupMessage(_buffer, 0U, TP_PAYLOAD_MAX_SIZE, true);
    receiver.start(_channel, _message, _listener);
    result = receiver.receive(_channel, _message, timestamp);
    EXPECT_EQ(TpReceiver::TpResult::TP_PENDING, result);

    size_t bytesRemaining = payloadLength - TP_PAYLOAD_MAX_SIZE;
    while (bytesRemaining > TP_PAYLOAD_MAX_SIZE)
    {
        setupMessage(
            _buffer,
            payloadLength - bytesRemaining,
            bytesRemaining > TP_PAYLOAD_MAX_SIZE ? TP_PAYLOAD_MAX_SIZE : bytesRemaining,
            true);
        bytesRemaining -= TP_PAYLOAD_MAX_SIZE;
        timestamp++;

        result = receiver.receive(_channel, _message, timestamp);
        EXPECT_EQ(TpReceiver::TpResult::TP_PENDING, result);
    }
    setupMessage(_buffer, payloadLength - bytesRemaining, bytesRemaining, false);
    timestamp++;
    EXPECT_CALL(_listener, receivedTpMessage(Ref(_channel), _)).Times(1U);

    result = receiver.receive(_channel, _message, timestamp);
    EXPECT_EQ(TpReceiver::TpResult::TP_OK, result);

    verifyMessage(buffer, payloadLength);

    EXPECT_TRUE(receiver.isActive());
    receiver.stop();
}

/**
 * Test successfully receiving a small message.
 */
TEST_F(TpReceiverTest, successfully_receive_small_message)
{
    size_t const payloadLength = UDP_PAYLOAD_MAX_SIZE;

    ::etl::array<uint8_t, payloadLength + SomeIpConstants::HEADER_LENGTH> buffer{};
    TestTpReceiver receiver(buffer);

    setupMessage(_buffer, 0U, payloadLength, false);
    receiver.start(_channel, _message, _listener);

    TpReceiver::TpResult result;
    uint32_t timestamp = 1U;

    EXPECT_CALL(_listener, receivedTpMessage(Ref(_channel), _)).Times(1U);

    result = receiver.receive(_channel, _message, timestamp);
    EXPECT_EQ(TpReceiver::TpResult::TP_OK, result);

    verifyMessage(buffer, payloadLength);

    EXPECT_TRUE(receiver.isActive());
    receiver.stop();
}

/**
 * Test successfully receiving an empty message.
 */
TEST_F(TpReceiverTest, successfully_receive_empty_message)
{
    size_t const payloadLength = 0U;

    ::etl::array<uint8_t, SomeIpConstants::HEADER_LENGTH> buffer{};
    TestTpReceiver receiver(buffer);

    setupMessage(_buffer, 0U, payloadLength, false);
    receiver.start(_channel, _message, _listener);

    TpReceiver::TpResult result;
    uint32_t timestamp = 1U;

    EXPECT_CALL(_listener, receivedTpMessage(Ref(_channel), _)).Times(1U);

    result = receiver.receive(_channel, _message, timestamp);
    EXPECT_EQ(TpReceiver::TpResult::TP_OK, result);

    verifyMessage(buffer, payloadLength);

    EXPECT_TRUE(receiver.isActive());
    receiver.stop();
}

TEST_F(TpReceiverTest, expired)
{
    size_t const payloadLength = TOTAL_MAX_TP_PAYLOAD_SIZE;

    ::someip::declare::TpReceiver<payloadLength + ::someip::SomeIpConstants::HEADER_LENGTH>
        receiver;
    TpReceiver::TpResult result;

    setupMessage(_buffer, 0U, payloadLength - 20U /* tp-header */, true);

    receiver.start(_channel, _message, _listener);
    uint32_t const timestamp = 1U;

    result = receiver.receive(_channel, _message, timestamp);
    EXPECT_EQ(TpReceiver::TpResult::TP_PENDING, result);

    EXPECT_FALSE(receiver.isExpired(timestamp));
    EXPECT_FALSE(receiver.isExpired(timestamp + ITpTransceiver::TP_RECEIVE_TIMEOUT - 1U));
    EXPECT_TRUE(receiver.isExpired(timestamp + ITpTransceiver::TP_RECEIVE_TIMEOUT));

    EXPECT_TRUE(receiver.isActive());
    receiver.stop();
}

/**
 * Make sure receiving is unsuccessful if buffer is too small.
 */
TEST_F(TpReceiverTest, buffer_too_small)
{
    size_t const payloadLength = TOTAL_MAX_TP_PAYLOAD_SIZE;

    ::etl::array<uint8_t, payloadLength + SomeIpConstants::HEADER_LENGTH - 1U> buffer{};
    TestTpReceiver receiver(buffer);

    TpReceiver::TpResult result;
    uint32_t timestamp = 1U;

    setupMessage(_buffer, 0U, TP_PAYLOAD_MAX_SIZE, true);
    receiver.start(_channel, _message, _listener);
    result = receiver.receive(_channel, _message, timestamp);
    EXPECT_EQ(TpReceiver::TpResult::TP_PENDING, result);

    size_t bytesRemaining = payloadLength - TP_PAYLOAD_MAX_SIZE;
    while (bytesRemaining > TP_PAYLOAD_MAX_SIZE)
    {
        setupMessage(
            _buffer,
            payloadLength - bytesRemaining,
            bytesRemaining > TP_PAYLOAD_MAX_SIZE ? TP_PAYLOAD_MAX_SIZE : bytesRemaining,
            true);
        bytesRemaining -= TP_PAYLOAD_MAX_SIZE;
        timestamp++;

        result = receiver.receive(_channel, _message, timestamp);
        EXPECT_EQ(TpReceiver::TpResult::TP_PENDING, result);
    }
    setupMessage(_buffer, payloadLength - bytesRemaining, bytesRemaining, false);
    timestamp++;
    result = receiver.receive(_channel, _message, timestamp);
    EXPECT_EQ(TpReceiver::TpResult::TP_ERROR, result);

    EXPECT_TRUE(receiver.isActive());
    receiver.stop();
}

/**
 * Make sure receiving is unsuccessful if header is incomplete.
 */
TEST_F(TpReceiverTest, incomplete_header)
{
    size_t const payloadLength = 0U;

    ::someip::declare::TpReceiver<payloadLength + SomeIpConstants::HEADER_LENGTH> receiver;

    setupMessage(_buffer, 0U, payloadLength, false);
    _message.setPayloadLength(0U); // no tp-header

    receiver.start(_channel, _message, _listener);

    TpReceiver::TpResult result;
    result = receiver.receive(_channel, _message, 1U);
    EXPECT_EQ(TpReceiver::TpResult::TP_ERROR, result);

    EXPECT_TRUE(receiver.isActive());
    receiver.stop();
}

} // anonymous namespace
