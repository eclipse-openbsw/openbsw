/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/TpTransceiver.h"

#include "bsp/timer/SystemTimerMock.h"
#include "gmock/gmock.h"
#include "someip/ITpTransceiver.h"
#include "someip/NetworkChannel.h"
#include "someip/NetworkResourceMock.h"
#include "someip/SomeIpConstants.h"
#include "someip/SomeIpMessage.h"
#include "someip/TpConfig.h"
#include "someip/TpListenerMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <etl/unaligned_type.h>
#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::ip;
using namespace ::someip;

struct TpTransceiverTest : Test
{
    TpTransceiverTest() : _asyncMock(), _testContext(_ethernetContext)
    {
        _testContext.handleAll();
        _resource.incRefCounter();
        EXPECT_CALL(_resource, isOpen()).Times(AnyNumber()).WillRepeatedly(Return(true));
        EXPECT_CALL(_resource, isConnected()).Times(AnyNumber()).WillRepeatedly(Return(false));
        EXPECT_CALL(_resource, getLocalPort()).Times(AnyNumber()).WillRepeatedly(Return(15U));
    }

    static void setupOutgoingMessage(SomeIpMessage& message, size_t payloadLength)
    {
        message.setMessageId(1U);
        message.setMessageType(SomeIpMessage::MessageType::REQUEST);
        message.setPayloadLength(payloadLength);
    }

    static void setupIncomingMessage(
        SomeIpMessage& message,
        size_t payloadOffset,
        size_t chunkLength,
        bool hasMoreSegments,
        uint32_t messageId = 1U)
    {
        message.setMessageId(messageId);
        message.setRawMessageType(
            static_cast<uint8_t>(SomeIpMessage::MessageType::REQUEST)
            | ITpTransceiver::TP_MESSAGE_TYPE_BIT_MASK);
        message.setPayloadLength(chunkLength + ITpTransceiver::TP_HEADER_LENGTH);

        ITpTransceiver::TpHeader tpHeader{};
        tpHeader.payloadOffset   = payloadOffset;
        tpHeader.hasMoreSegments = hasMoreSegments;

        etl::be_uint32_ext_t{message.getPayload()} = ITpTransceiver::serializeTpHeader(tpHeader);
    }

    StrictMock<SystemTimerMock> _stm;

    StrictMock<NetworkResourceMock> _resource;
    IPEndpoint _address{make_ip4(192U, 0U, 2U, 1U), 15U};
    NetworkChannel _channel{_resource, _address};

    async::ContextType _ethernetContext{0U};
    ::someip::internal::TpResources<1U, UDP_PACKET_MAX_SIZE + 20U /* tp-header */> _config;
    TpTransceiver _transceiver{_ethernetContext, _config.tpSenders, _config.tpReceivers};

    StrictMock<TpListenerMock> _listener;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _testContext;
};

/**
 * Make sure UDP and TP constraints are met.
 */
TEST_F(TpTransceiverTest, test_constraints)
{
    EXPECT_TRUE(UDP_PACKET_MAX_SIZE > UDP_PAYLOAD_MAX_SIZE);
    EXPECT_TRUE(UDP_PAYLOAD_MAX_SIZE > TP_PAYLOAD_MAX_SIZE);
    EXPECT_TRUE((TP_PAYLOAD_MAX_SIZE % 0x10U) == 0U); // multiple of 16 !

    EXPECT_TRUE(ITpTransceiver::TP_UPDATE_CYCLE < ITpTransceiver::TP_RECEIVE_TIMEOUT);
}

/**
 * Make sure isOutgoingTpMessage() is only successful if proto == SD_L4_PROTO_UDP and length >
 * UDP_PACKET_MAX_SIZE. isIncomingTpMessage() is only successful if proto == SD_L4_PROTO_UDP as well
 * and type is TP_MESSAGE_TYPE_BIT_MASK.
 */
TEST_F(TpTransceiverTest, test_helper)
{
    EXPECT_TRUE(
        ITpTransceiver::isOutgoingTpMessage(proto::SD_L4_PROTO_UDP, UDP_PACKET_MAX_SIZE + 1U));
    EXPECT_FALSE(ITpTransceiver::isOutgoingTpMessage(proto::SD_L4_PROTO_UDP, UDP_PACKET_MAX_SIZE));
    EXPECT_FALSE(
        ITpTransceiver::isOutgoingTpMessage(proto::SD_L4_PROTO_TCP, UDP_PACKET_MAX_SIZE + 1U));

    EXPECT_TRUE(ITpTransceiver::isIncomingTpMessage(
        proto::SD_L4_PROTO_UDP,
        static_cast<SomeIpMessage::MessageType>(
            static_cast<uint8_t>(SomeIpMessage::MessageType::REQUEST)
            | ITpTransceiver::TP_MESSAGE_TYPE_BIT_MASK)));
    EXPECT_FALSE(ITpTransceiver::isIncomingTpMessage(
        proto::SD_L4_PROTO_UDP, SomeIpMessage::MessageType::REQUEST));
    EXPECT_FALSE(ITpTransceiver::isIncomingTpMessage(
        proto::SD_L4_PROTO_TCP,
        static_cast<SomeIpMessage::MessageType>(
            static_cast<uint8_t>(SomeIpMessage::MessageType::REQUEST)
            | ITpTransceiver::TP_MESSAGE_TYPE_BIT_MASK)));
}

TEST_F(TpTransceiverTest, header)
{
    ITpTransceiver::TpHeader header1{};
    header1.payloadOffset   = 3735941120U;
    header1.hasMoreSegments = true;

    uint32_t value1 = 0xDEADF001U;
    EXPECT_EQ(value1, ITpTransceiver::serializeTpHeader(header1));

    ITpTransceiver::TpHeader header2{};
    ITpTransceiver::parseTpHeader(value1, header2);

    EXPECT_EQ(3735941120U, header2.payloadOffset);
    EXPECT_TRUE(header2.hasMoreSegments);

    header2.hasMoreSegments = false;

    uint32_t value2 = 0xDEADF000U;
    EXPECT_EQ(value2, ITpTransceiver::serializeTpHeader(header2));

    ITpTransceiver::TpHeader header3{};
    ITpTransceiver::parseTpHeader(value2, header3);

    EXPECT_EQ(3735941120U, header3.payloadOffset);
    EXPECT_FALSE(header3.hasMoreSegments);
}

/**
 * Test successfully sending TP message.
 */
TEST_F(TpTransceiverTest, send_successfully)
{
    for (int i = 0; i < 2; ++i)
    {
        size_t const payloadLength = UDP_PACKET_MAX_SIZE;
        ::etl::array<uint8_t, payloadLength + 16U /* msg-header */> buffer{};

        SomeIpMessage message(buffer);
        setupOutgoingMessage(message, payloadLength);

        EXPECT_CALL(_resource, send(_, _)).Times(2).WillRepeatedly(Return(true));
        EXPECT_TRUE(_transceiver.sendTpMessage(_channel, message));
    }
}

TEST_F(TpTransceiverTest, sendMessageFailure)
{
    size_t const payloadLength = UDP_PACKET_MAX_SIZE;
    ::etl::array<uint8_t, payloadLength + 16U /* msg-header */> buffer{};

    SomeIpMessage message(buffer);
    setupOutgoingMessage(message, payloadLength);

    EXPECT_CALL(_resource, send(_, _)).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(_transceiver.sendTpMessage(_channel, message));
}

/**
 * Test receiving TP message successfully.
 */
TEST_F(TpTransceiverTest, receive_successfully)
{
    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    for (int i = 0; i < 2; ++i)
    {
        size_t const payloadLength = UDP_PACKET_MAX_SIZE;
        ::etl::array<uint8_t, payloadLength> buffer{};

        SomeIpMessage message(buffer);
        setupIncomingMessage(message, 0U, payloadLength - 20U, true);

        // EXPECT_CALL(_timeoutManager, set(Ref(_transceiver), ITpTransceiver::TP_UPDATE_CYCLE,
        // false))
        //     .Times(1)
        //     .WillOnce(Return(ITimeoutManager2::TIMEOUT_OK));
        _transceiver.receiveTpMessage(_channel, message, _listener);

        setupIncomingMessage(message, payloadLength - 20U, 20U, false);

        // EXPECT_CALL(_timeoutManager, cancel(Ref(_transceiver))).Times(1);
        EXPECT_CALL(_listener, receivedTpMessage(Ref(_channel), _)).Times(1U);
        _transceiver.receiveTpMessage(_channel, message, _listener);
    }
}

TEST_F(TpTransceiverTest, receivedMessageFailureWhileIdle)
{
    size_t const payloadLength = UDP_PACKET_MAX_SIZE;
    ::etl::array<uint8_t, payloadLength> buffer{};

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    SomeIpMessage message(buffer);
    setupIncomingMessage(message, 0U, payloadLength - 20U, true);
    message.setPayloadLength(0U); // invalid

    _transceiver.receiveTpMessage(_channel, message, _listener);
}

TEST_F(TpTransceiverTest, receivedMessageFailureWhilePending)
{
    size_t const payloadLength = UDP_PACKET_MAX_SIZE;
    ::etl::array<uint8_t, payloadLength> buffer{};

    SomeIpMessage message(buffer);
    setupIncomingMessage(message, 0U, payloadLength - 20U, true);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    // EXPECT_CALL(_timeoutManager, set(Ref(_transceiver), ITpTransceiver::TP_UPDATE_CYCLE, false))
    //     .Times(1)
    //     .WillOnce(Return(ITimeoutManager2::TIMEOUT_OK));
    _transceiver.receiveTpMessage(_channel, message, _listener);

    setupIncomingMessage(message, payloadLength - 20U, 20U, false);
    message.setPayloadLength(0U); // invalid

    // EXPECT_CALL(_timeoutManager, cancel(Ref(_transceiver))).Times(1);
    _transceiver.receiveTpMessage(_channel, message, _listener);
}

TEST_F(TpTransceiverTest, receivedMessageBusy)
{
    size_t const payloadLength = UDP_PACKET_MAX_SIZE;
    ::etl::array<uint8_t, payloadLength> buffer1{};
    ::etl::array<uint8_t, payloadLength> buffer2{};

    SomeIpMessage message1(buffer1);
    SomeIpMessage message2(buffer2);

    setupIncomingMessage(message1, 0U, payloadLength - 20U, true, 1U);
    setupIncomingMessage(message2, 0U, payloadLength - 20U, true, 2U);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    // EXPECT_CALL(_timeoutManager, set(Ref(_transceiver), ITpTransceiver::TP_UPDATE_CYCLE, false))
    //     .Times(1)
    //     .WillOnce(Return(ITimeoutManager2::TIMEOUT_OK));
    _transceiver.receiveTpMessage(_channel, message1, _listener);

    _transceiver.receiveTpMessage(_channel, message2, _listener); // nothing

    setupIncomingMessage(message1, payloadLength - 20U, 20U, false);
    setupIncomingMessage(message2, payloadLength - 20U, 20U, false);

    // EXPECT_CALL(_timeoutManager, cancel(Ref(_transceiver))).Times(1);
    EXPECT_CALL(_listener, receivedTpMessage(Ref(_channel), _)).Times(1U);
    _transceiver.receiveTpMessage(_channel, message1, _listener);

    // EXPECT_CALL(_timeoutManager, set(Ref(_transceiver), ITpTransceiver::TP_UPDATE_CYCLE, false))
    //     .Times(1)
    //     .WillOnce(Return(ITimeoutManager2::TIMEOUT_OK));
    _transceiver.receiveTpMessage(_channel, message2, _listener); // pending

    // EXPECT_CALL(_timeoutManager, cancel(Ref(_transceiver))).Times(1); // cleanup
    _transceiver.stop();
}

} // anonymous namespace
