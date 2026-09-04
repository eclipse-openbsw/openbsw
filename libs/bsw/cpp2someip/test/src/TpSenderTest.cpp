/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TpSender.h"

#include "someip/ITpTransceiver.h"
#include "someip/NetworkChannel.h"
#include "someip/NetworkResourceMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"

#include <etl/unaligned_type.h>
#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;

class TestTpSender : public TpSender
{
public:
    explicit TestTpSender(::etl::span<uint8_t> const& buffer) : TpSender(buffer) {}
};

struct TpSenderTest : Test
{
    TpSenderTest()
    {
        _resource.incRefCounter();
        EXPECT_CALL(_resource, isOpen()).Times(AnyNumber()).WillRepeatedly(Return(true));
        EXPECT_CALL(_resource, isConnected()).Times(AnyNumber()).WillRepeatedly(Return(false));
    }

    void setupMessage(size_t payloadLength)
    {
        EXPECT_LE(payloadLength + 16U, _buffer.size());
        SomeIpMessage message(_buffer);
        message.setMessageId(1U);
        message.setRequestId(2U);
        message.setProtocolVersion(3U);
        message.setInterfaceVersion(4U);
        message.setMessageType(SomeIpMessage::MessageType::REQUEST);
        message.setReturnCode(static_cast<SomeIpMessage::ReturnCode>(5U));
        message.setPayloadLength(payloadLength);

        for (size_t i = 0U; i < payloadLength; ++i)
        {
            _buffer.at(SomeIpMessage::OFFSET_PAYLOAD + i) = (i % 255U);
        }
    }

    static void verifyMessage(::etl::span<uint8_t> const& buffer, size_t payloadLength)
    {
        SomeIpMessage message(buffer);
        ::etl::span<uint8_t const> payload = message.getBufferPayload();

        EXPECT_EQ(1U, message.getMessageId());
        EXPECT_EQ(2U, message.getRequestId());
        EXPECT_EQ(3U, message.getProtocolVersion());
        EXPECT_EQ(4U, message.getInterfaceVersion());
        EXPECT_EQ(
            static_cast<uint8_t>(SomeIpMessage::MessageType::REQUEST)
                | ITpTransceiver::TP_MESSAGE_TYPE_BIT_MASK,
            message.getRawMessageType());
        EXPECT_EQ(static_cast<SomeIpMessage::ReturnCode>(5U), message.getReturnCode());

        ITpTransceiver::TpHeader tpHeader{};
        ITpTransceiver::parseTpHeader(etl::be_uint32_t{&payload[0U]}, tpHeader);

        EXPECT_FALSE(tpHeader.hasMoreSegments);

        size_t chunkLength = payloadLength - tpHeader.payloadOffset;
        EXPECT_EQ(chunkLength, message.getPayloadLength() - ITpTransceiver::TP_HEADER_LENGTH);

        for (size_t i = tpHeader.payloadOffset; i < payloadLength; ++i)
        {
            EXPECT_EQ(
                (i % 255U),
                payload.at(i - tpHeader.payloadOffset + ITpTransceiver::TP_HEADER_LENGTH));
        }
    }

    static size_t const TP_HEADER_SIZE   = 20U;
    static size_t const MAX_MESSAGE_SIZE = 2048;
    StrictMock<NetworkResourceMock> _resource;
    IPEndpoint _address{make_ip4(192U, 0U, 2U, 1U), 15U};
    NetworkChannel _channel{_resource, _address};

    ::etl::array<uint8_t, MAX_MESSAGE_SIZE /* msg-header */> _buffer{};
    SomeIpMessage _message{_buffer};
};

/**
 * Test successfully sending message.
 */
TEST_F(TpSenderTest, send_message_successfully)
{
    setupMessage(UDP_PACKET_MAX_SIZE);
    ::someip::declare::TpSender<UDP_PACKET_MAX_SIZE> sender;

    EXPECT_CALL(_resource, send(_, _)).Times(2).WillRepeatedly(Return(true));

    TpSender::TpResult result = sender.send(_channel, _message);
    EXPECT_EQ(TpSender::TpResult::TP_OK, result);
}

/**
 * Test successfully sending a large message.
 */
TEST_F(TpSenderTest, successfully_send_large_message)
{
    size_t const expectedTailSize = 40U;
    size_t const payloadLength    = TP_PAYLOAD_MAX_SIZE + expectedTailSize;
    setupMessage(payloadLength);

    ::etl::array<uint8_t, UDP_PACKET_MAX_SIZE> buffer{};
    TestTpSender sender(buffer);

    // chunk-1 is tp-header (20 bytes) + (payload - tp-header)

    EXPECT_CALL(_resource, send(_, TP_PAYLOAD_MAX_SIZE + TP_HEADER_SIZE))
        .Times(1)
        .WillOnce(Return(true));

    // chunk-2 is tp-header (20 bytes) + rest of payload (40 bytes)
    EXPECT_CALL(_resource, send(_, expectedTailSize + TP_HEADER_SIZE))
        .Times(1)
        .WillOnce(Return(true));

    TpSender::TpResult result = sender.send(_channel, _message);
    EXPECT_EQ(TpSender::TpResult::TP_OK, result);

    verifyMessage(buffer, payloadLength);
}

/**
 * Test successfully sending a small message.
 */
TEST_F(TpSenderTest, successfully_send_small_message)
{
    size_t const payloadLength = TP_PAYLOAD_MAX_SIZE / 2U;
    setupMessage(payloadLength);

    ::etl::array<uint8_t, UDP_PACKET_MAX_SIZE> buffer{};
    TestTpSender sender(buffer);

    // chunk-1 is tp-header (20 bytes) + payload
    EXPECT_CALL(_resource, send(_, payloadLength + TP_HEADER_SIZE)).Times(1).WillOnce(Return(true));

    TpSender::TpResult result = sender.send(_channel, _message);
    EXPECT_EQ(TpSender::TpResult::TP_OK, result);

    verifyMessage(buffer, payloadLength);
}

/**
 * Test successfully sending an empty message.
 */
TEST_F(TpSenderTest, successfully_send_empty_message)
{
    size_t const payloadLength = 0U;
    setupMessage(payloadLength);

    ::etl::array<uint8_t, UDP_PACKET_MAX_SIZE> buffer{};
    TestTpSender sender(buffer);

    // chunk-1 is tp-header (20 bytes)
    EXPECT_CALL(_resource, send(_, TP_HEADER_SIZE)).Times(1).WillOnce(Return(true));

    TpSender::TpResult result = sender.send(_channel, _message);
    EXPECT_EQ(TpSender::TpResult::TP_OK, result);

    verifyMessage(buffer, payloadLength);
}

TEST_F(TpSenderTest, sendMessageFailed)
{
    setupMessage(UDP_PACKET_MAX_SIZE);
    ::someip::declare::TpSender<UDP_PACKET_MAX_SIZE> sender;

    EXPECT_CALL(_resource, send(_, _)).Times(1).WillOnce(Return(false));

    TpSender::TpResult result = sender.send(_channel, _message);
    EXPECT_EQ(TpSender::TpResult::TP_ERROR, result);
}

/**
 * Make sure sending is unsuccessful if buffer is too small.
 */
TEST_F(TpSenderTest, buffer_too_small)
{
    setupMessage(UDP_PACKET_MAX_SIZE);
    ::someip::declare::TpSender<UDP_PACKET_MAX_SIZE - 1U> sender;

    TpSender::TpResult result = sender.send(_channel, _message);
    EXPECT_EQ(TpSender::TpResult::TP_ERROR, result);
}

} // anonymous namespace
