/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/BufferedEventSender.h"

#include "bsp/timer/SystemTimerMock.h"
#include "gmock/gmock.h"
#include "someip/EventMessage.h"
#include "someip/NetworkMock.h"
#include "someip/NetworkResourceMock.h"
#include "someip/SomeIpMessage.h"
#include "someip/SomeIpSerializableBufferMock.h"
#include "someip/TpTransceiverMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;
using namespace ::common;
using namespace ::someip;
using namespace ::ip;

struct BufferedEventSenderTest : Test
{
    BufferedEventSenderTest()
    : _eventSender(_network, _ethernetContext, _tpTransceiver)
    , _endpoint(make_ip4(192U, 0U, 2U, 0U), 10U)
    , _asyncMock()
    , _testContext(_ethernetContext)
    {
        _networkResource.incRefCounter();
        _testContext.handleAll();
    }

    static uint8_t const NUM_OF_EVENT_BUFFERS = 8U;

    void setupSendDataReturnTrue(
        IPEndpoint const& endpoint, uint16_t port, uint8_t proto, uint16_t length = 0U);

    void setupSendDataReturnFalse(IPEndpoint const& endpoint, uint16_t port, uint8_t proto);

    StrictMock<NetworkResourceMock> _networkResource;
    ::etl::optional<NetworkChannel> _networkChannel;
    StrictMock<NetworkMock> _network;
    StrictMock<TpTransceiverMock> _tpTransceiver;
    async::ContextType _ethernetContext{0U};
    ::someip::declare::BufferedEventSender<NUM_OF_EVENT_BUFFERS> _eventSender;
    StrictMock<SystemTimerMock> _stm;
    IPEndpoint _endpoint;
    ::testing::StrictMock<::async::AsyncMock> _asyncMock;
    ::async::TestContext _testContext;
};

void BufferedEventSenderTest::setupSendDataReturnTrue(
    IPEndpoint const& /*endpoint*/, uint16_t /*port*/, uint8_t /*proto*/, uint16_t length)
{
    EXPECT_CALL(_networkResource, isOpen()).Times(AnyNumber()).WillRepeatedly(Return(true));
    EXPECT_CALL(_networkResource, isConnected()).Times(AnyNumber()).WillRepeatedly(Return(false));

    if (length == 0U)
    {
        EXPECT_CALL(_networkResource, send(_, _)).Times(1).WillOnce(Return(true));
    }
    else if (length <= UDP_PACKET_MAX_SIZE)
    {
        EXPECT_CALL(_networkResource, send(_, length)).Times(1).WillOnce(Return(true));
    }
    else
    {
        EXPECT_CALL(_tpTransceiver, sendTpMessage(_, _)).WillOnce(Return(true));
    }
}

void BufferedEventSenderTest::setupSendDataReturnFalse(
    IPEndpoint const& endpoint, uint16_t port, uint8_t proto)
{
    _networkChannel.reset();

    EXPECT_CALL(_network, getRpcChannel(port, endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));
}

/**
 * Tests that TimeoutManager is canceled after shutting down EventSender.
 */
TEST_F(BufferedEventSenderTest, TimeoutManager_needs_to_be_canceled_after_shutting_down_EventSender)
{
    _eventSender.init();

    // EXPECT_CALL(_timeoutManager, cancel(_)).Times(1);
    _eventSender.shutdown();
}

/**
 * Tests sending buffered events with payload.
 */
TEST_F(BufferedEventSenderTest, send_buffered_event_with_payload)
{
    uint32_t const delayTime = 100U;

    uint16_t port       = 20U;
    uint8_t const proto = 0x11;

    EXPECT_EQ(0U, _eventSender.countMessages());

    _networkChannel = NetworkChannel(_networkResource, _endpoint);

    uint8_t const data[] = {0x01, 0x02, 0x03};
    ::etl::span<uint8_t const> const buffer(data);

    SomeIpSerializableBufferMock payload(&buffer);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    IEventSender::ErrorCode const errorCode
        = _eventSender.sendEvent(0x01, 0x02, 0x03, delayTime, &payload, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
    EXPECT_EQ(1U, _eventSender.countMessages());
}

/**
 * Tests sending buffered events without payload.
 */
TEST_F(BufferedEventSenderTest, send_buffered_event_without_payload)
{
    uint32_t const delayTime = 100U;

    uint16_t port       = 20U;
    uint8_t const proto = 0x11;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit());

    EXPECT_EQ(0U, _eventSender.countMessages());

    _networkChannel = NetworkChannel(_networkResource, _endpoint);

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    IEventSender::ErrorCode const errorCode
        = _eventSender.sendEvent(0x01, 0x02, 0x03, delayTime, nullptr, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
    EXPECT_EQ(1U, _eventSender.countMessages());
}

TEST_F(BufferedEventSenderTest, SendEvent_duplicate_pdu)
{
    uint16_t const serviceId = 0x01;
    uint8_t const version    = 0x02;
    uint16_t const port      = 20U;
    uint8_t const proto      = 0x11;
    uint32_t const delayTime = 100U;

    uint16_t const event1Id       = 0x03;
    uint8_t const event1Payload[] = {0xA1, 0xA2, 0xA3};
    ::etl::span<uint8_t const> const event1Buffer(event1Payload);

    uint16_t const event2Id       = 0x04;
    uint8_t const event2Payload[] = {0xB1, 0xB2, 0xB3, 0xB4, 0xB5};
    ::etl::span<uint8_t const> const event2Buffer(event2Payload);

    uint8_t const event3Payload[] = {0xC1, 0xC2};
    ::etl::span<uint8_t const> const event3Buffer(event3Payload);

    _networkChannel = NetworkChannel(_networkResource, _endpoint);

    SomeIpSerializableBufferMock event1(&event1Buffer);

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    IEventSender::ErrorCode errorCode = _eventSender.sendEvent(
        serviceId, version, event1Id, delayTime, &event1, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);

    SomeIpSerializableBufferMock event2(&event2Buffer);

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    errorCode = _eventSender.sendEvent(
        serviceId, version, event2Id, delayTime, &event2, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);

    // duplicate pdu triggers send of buffered events 1 and 2
    uint16_t const length
        = SomeIpConstants::HEADER_LENGTH * 2U + sizeof(event1Payload) + sizeof(event2Payload);

    setupSendDataReturnTrue(_endpoint, port, proto, length);

    SomeIpSerializableBufferMock event3(&event3Buffer);
    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(2)
        .WillRepeatedly(Return(_networkChannel));

    errorCode = _eventSender.sendEvent(
        serviceId, version, event1Id, delayTime, &event3, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
    EXPECT_EQ(1U, _eventSender.countMessages()); // remaining event 3
}

/**
 * Tests sending buffered events with payload which is too big.
 */
TEST_F(BufferedEventSenderTest, send_buffered_event_with_too_big_payload)
{
    uint8_t huge_payload_size
        [::someip::internal::EventMessage::BUFFER_SIZE - SomeIpMessage::OFFSET_PAYLOAD + 1U];
    ::etl::span<uint8_t const> const buffer(huge_payload_size);

    uint16_t const port = 20U;
    uint8_t const proto = 0x11;

    uint16_t const length = SomeIpConstants::HEADER_LENGTH + buffer.size();
    setupSendDataReturnTrue(_endpoint, port, proto, length);
    _networkChannel = NetworkChannel(_networkResource, _endpoint);

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    SomeIpSerializableBufferMock payload(&buffer);

    IEventSender::ErrorCode const errorCode
        = _eventSender.sendEvent(0x01, 0x02, 0x03, 1U, &payload, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
}

/**
 * Tests sending buffered events with zero delay time.
 */
TEST_F(BufferedEventSenderTest, send_buffered_event_with_zero_delay_time)
{
    uint8_t const data[] = {0x01, 0x02, 0x03};
    ::etl::span<uint8_t const> const buffer(data);

    uint16_t const port = 20U;
    uint8_t const proto = 0x11;

    uint16_t const length = SomeIpConstants::HEADER_LENGTH + buffer.size();
    setupSendDataReturnTrue(_endpoint, port, proto, length);
    _networkChannel = NetworkChannel(_networkResource, _endpoint);

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    SomeIpSerializableBufferMock payload(&buffer);

    IEventSender::ErrorCode const errorCode
        = _eventSender.sendEvent(0x01, 0x02, 0x03, 0U, &payload, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
}

/**
 * Tests sending empty buffered events with zero delay time.
 */
TEST_F(BufferedEventSenderTest, send_empty_buffered_event_with_zero_delay_time)
{
    uint16_t const port = 20U;
    uint8_t const proto = 0x11;

    uint16_t const length = SomeIpConstants::HEADER_LENGTH;
    setupSendDataReturnTrue(_endpoint, port, proto, length);
    _networkChannel = NetworkChannel(_networkResource, _endpoint);

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    IEventSender::ErrorCode const errorCode
        = _eventSender.sendEvent(0x01, 0x02, 0x03, 0U, nullptr, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
}

/**
 * Tests sending buffered event fails. Behaviour is the same if there is no channel.
 */
TEST_F(BufferedEventSenderTest, sending_buffered_event_fails)
{
    uint8_t const data[] = {0x01, 0x02, 0x03};
    ::etl::span<uint8_t const> const buffer(data);
    uint16_t const port = 20U;
    uint8_t const proto = 0x11;
    setupSendDataReturnFalse(_endpoint, port, proto);
    SomeIpSerializableBufferMock payload(&buffer);

    IEventSender::ErrorCode const errorCode
        = _eventSender.sendEvent(0x01, 0x02, 0x03, 0U, &payload, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_ERROR, errorCode);
}

TEST_F(BufferedEventSenderTest, SendEvent_EndpointAlreadyAdded)
{
    uint8_t const data[] = {0x01, 0x02, 0x03};
    ::etl::span<uint8_t const> const buffer(data);
    uint8_t const data1[] = {0x04, 0x05, 0x06, 0x07};
    ::etl::span<uint8_t const> const buffer1(data1);
    uint32_t const delayTime = 100U;

    uint16_t port       = 20U;
    uint8_t const proto = 0x11;

    EXPECT_EQ(0U, _eventSender.countMessages());
    _networkChannel = NetworkChannel(_networkResource, _endpoint);

    SomeIpSerializableBufferMock payload(&buffer);

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    IEventSender::ErrorCode errorCode
        = _eventSender.sendEvent(0x01, 0x02, 0x03, delayTime, &payload, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
    EXPECT_EQ(1U, _eventSender.countMessages());

    setupSendDataReturnTrue(_endpoint, port, proto);

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(2)
        .WillRepeatedly(Return(_networkChannel));

    // but payload1 won't be sent.
    SomeIpSerializableBufferMock payload1(&buffer1);

    errorCode
        = _eventSender.sendEvent(0x01, 0x02, 0x03, delayTime, &payload1, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
    EXPECT_EQ(1U, _eventSender.countMessages());
}

TEST_F(BufferedEventSenderTest, SendEvent_MaxEndpoints)
{
    uint8_t const proto = 0x11;
    uint8_t maxBuffers  = NUM_OF_EVENT_BUFFERS;

    EXPECT_CALL(_stm, getSystemTimeMs32Bit()).Times(AnyNumber());

    for (uint8_t i = 0U; i < maxBuffers; ++i)
    {
        EXPECT_EQ(i, _eventSender.countMessages());

        uint16_t port = 20U + i;

        uint8_t const data[] = {0x01, 0x02, uint8_t(0x03 + i)};
        ::etl::span<uint8_t const> const buffer(data);

        SomeIpSerializableBufferMock payload(&buffer);
        _networkChannel = NetworkChannel(_networkResource, _endpoint);

        EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
            .Times(1)
            .WillOnce(Return(_networkChannel));

        IEventSender::ErrorCode const errorCode = _eventSender.sendEvent(
            1U + i, 0x02, 2U + i, i + 1U, &payload, port, proto, _endpoint);

        EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
        EXPECT_EQ(i + 1U, _eventSender.countMessages());
    }
    // Add one more endpoint, should force it to send the buffer
    uint16_t port = 50U;

    setupSendDataReturnTrue(_endpoint, port, proto);
    _networkChannel = NetworkChannel(_networkResource, _endpoint);

    EXPECT_CALL(_network, getRpcChannel(port, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));
    EXPECT_CALL(_network, getRpcChannel(20U, _endpoint, proto))
        .Times(1)
        .WillOnce(Return(_networkChannel));

    uint8_t const payload[] = {0x01, 0x02, 50U};
    ::etl::span<uint8_t const> const buffer(payload);

    SomeIpSerializableBufferMock event(&buffer);

    IEventSender::ErrorCode const errorCode
        = _eventSender.sendEvent(50U, 0x02, 52U, 51U, &event, port, proto, _endpoint);

    EXPECT_EQ(IEventSender::ErrorCode::EVENT_SEND_OK, errorCode);
    EXPECT_EQ(maxBuffers, _eventSender.countMessages());
}

} // anonymous namespace
