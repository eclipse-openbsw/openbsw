/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SomeIpMessage.h"

#include "someip/SomeIpConstants.h"

#include <etl/algorithm.h>
#include <etl/array.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;

struct SomeIpMessageTest : ::testing::Test
{};

struct ParameterizedUInt32Test : ::testing::TestWithParam<uint32_t>
{
    ParameterizedUInt32Test() : message(buffer) {}

    ::etl::array<uint8_t, SomeIpMessage::OFFSET_PAYLOAD> buffer{};
    SomeIpMessage message;
};

/**
 * Tests the behavior of the SomeIpMessage constructor when called with a valid pointer.
 */
TEST_F(SomeIpMessageTest, test_SomeIpMessage_constructor)
{
    ::etl::array<uint8_t, SomeIpMessage::OFFSET_PAYLOAD + 1U> buffer{};
    SomeIpMessage message(buffer);
    EXPECT_EQ(&buffer[SomeIpMessage::OFFSET_PAYLOAD], message.getPayload());
}

/**
 * Make sure getPayload() returns a pointer to the message after the header or nullptr.
 */
TEST_F(SomeIpMessageTest, GetPayload)
{
    ::etl::array<uint8_t, SomeIpMessage::OFFSET_PAYLOAD + 1U> buffer{};
    SomeIpMessage message(buffer);
    EXPECT_EQ(&buffer[SomeIpMessage::OFFSET_PAYLOAD], message.getPayload());

    ::etl::array<uint8_t, 5U> smallBuffer{};
    SomeIpMessage small(smallBuffer);
    EXPECT_EQ(nullptr, small.getPayload());
    EXPECT_EQ(&smallBuffer[0], small.getRawData());
}

TEST_F(SomeIpMessageTest, GetPayloadConst)
{
    ::etl::array<uint8_t, SomeIpMessage::OFFSET_PAYLOAD + 1U> buffer{};
    SomeIpMessage message(buffer);
    SomeIpMessage const& cMessage = message;

    EXPECT_EQ(&buffer[SomeIpMessage::OFFSET_PAYLOAD], cMessage.getPayload());

    ::etl::array<uint8_t, 5> smallBuffer{};
    SomeIpMessage small(smallBuffer);
    SomeIpMessage const& cSmall = small;
    EXPECT_EQ(nullptr, cSmall.getPayload());
}

/**
 * Test getting messageId. First to bytes of messageId are used as serviceId and next 2 bytes are
 * used as methodId.
 */
TEST_F(SomeIpMessageTest, test_getMessageId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD] = {0x12, 0x23, 0x34, 0x45};
    SomeIpMessage message(buffer);
    EXPECT_EQ(static_cast<uint32_t>(0x12233445), message.getMessageId());
    buffer[0] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFF233445), message.getMessageId());

    message.setServiceId(0xFFAA);
    message.setMethodId(0xBBFF);
    EXPECT_EQ(static_cast<uint32_t>(0xFFAABBFF), message.getMessageId());
    EXPECT_EQ(0U, message.getMaximumPayloadLength());
}

TEST_F(SomeIpMessageTest, PayloadLengthTest)
{
    ::etl::array<uint8_t, 255U> buf{};
    etl::fill(buf.begin(), buf.end(), 0xAB);

    SomeIpMessage message(buf);
    message.setServiceId(0x1234U);
    message.setMethodId(0x4567U);
    message.setRequestId(0U);
    message.setPayloadLength(0U);
    message.setProtocolVersion(someip::configuration::PROTOCOL_VERSION);
    message.setInterfaceVersion(0x89U);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_OK);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST_NO_RETURN);

    uint32_t length = message.getTotalLength();
    EXPECT_EQ(16U, length);
    for (uint32_t i = 0U; i < length; ++i)
    {
        EXPECT_NE(0xABU, buf[i]);
    }
    for (uint32_t i = length; i < 255U; ++i)
    {
        EXPECT_EQ(0xABU, buf[i]);
    }
}

struct ParameterizedSetMessageIdTest : public ParameterizedUInt32Test
{};

/**
 * Make sure different messageId values can be set correctly
 */
TEST_P(ParameterizedSetMessageIdTest, test_setMessageId_different_ids)
{
    message.setMessageId(GetParam());
    EXPECT_EQ(static_cast<uint32_t>(GetParam()), message.getMessageId());
}

INSTANTIATE_TEST_SUITE_P(
    test_setMessageId_different_ids,
    ParameterizedSetMessageIdTest,
    ::testing::Values(
        uint32_t(0),
        uint32_t(1),
        uint32_t(-1),
        uint32_t(0x12345678),
        uint32_t(0xFFFFFFFF),
        uint32_t(0xFEDCBA98)));

/**
 * Test getting serviceId.
 */
TEST_F(SomeIpMessageTest, test_getServiceId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD] = {0x12, 0x23, 0x34, 0x45};
    SomeIpMessage message(buffer);
    EXPECT_EQ(static_cast<uint32_t>(0x1223), message.getServiceId());
    buffer[0] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFF23), message.getServiceId());
    buffer[1] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getServiceId());
    buffer[2] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getServiceId());
    buffer[3] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getServiceId());
}

/**
 * \desc
 * Tests SomeIpMessage::setServiceId().
 */

/**
 * Test setting serviceId.
 */
TEST_F(SomeIpMessageTest, test_setServiceId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD] = {0x12, 0x23, 0x34, 0x45};
    SomeIpMessage message(buffer);
    message.setServiceId(0xFFFF);
    EXPECT_EQ(0xFFFF, message.getServiceId());
    message.setServiceId(0x0);
    EXPECT_EQ(0x0, message.getServiceId());
}

struct ParameterizedSetServiceIdTest : public ParameterizedUInt32Test
{};

/**
 * Make sure different serviceId values can be set correctly.
 */
TEST_P(ParameterizedSetServiceIdTest, test_setServiceId_with_different_ids)
{
    message.setServiceId(GetParam());
    EXPECT_EQ(static_cast<uint32_t>(GetParam()), message.getServiceId());
}

INSTANTIATE_TEST_SUITE_P(
    test_setServiceId_with_different_ids,
    ParameterizedSetServiceIdTest,
    ::testing::Values(uint32_t(0), uint32_t(1), uint32_t(0xFFFE), uint32_t(0xFFFF)));

/**
 * Test setting getting methodId.
 */
TEST_F(SomeIpMessageTest, test_getMethodId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD] = {0x12, 0x23, 0x34, 0x45};
    SomeIpMessage message(buffer);
    EXPECT_EQ(static_cast<uint32_t>(0x3445), message.getMethodId());
    buffer[2] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFF45), message.getMethodId());
    buffer[3] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getMethodId());
    buffer[0] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getMethodId());
    buffer[1] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getMethodId());
}

/**
 * Test setting methodId.
 */
TEST_F(SomeIpMessageTest, test_setMethodId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD] = {0x12, 0x23, 0x34, 0x45};
    SomeIpMessage message(buffer);
    message.setMethodId(0xFFFF);
    EXPECT_EQ(0xFFFF, message.getMethodId());
    message.setMethodId(0x0000);
    EXPECT_EQ(0x0, message.getMethodId());
}

struct ParameterizedSetMethodIdTest : public ParameterizedUInt32Test
{};

/**
 * Make sure different methodId values can be set correctly.
 */
TEST_P(ParameterizedSetMethodIdTest, test_setMethodId_with_different_ids)
{
    message.setMethodId(GetParam());
    EXPECT_EQ(GetParam(), message.getMethodId());
}

INSTANTIATE_TEST_SUITE_P(
    test_setMethodId_with_different_ids,
    ParameterizedSetMethodIdTest,
    ::testing::Values(
        uint32_t(0), uint32_t(1), uint32_t(0x0FF0), uint32_t(0x7FFE), uint32_t(0xFFFF)));

/**
 * Test getting serviceId and methodId after setting messageId.
 * The message id is divided into 16 bits of service id followed by one bit 0 and 15 bits method id.
 */
TEST_F(SomeIpMessageTest, test_setMessageId_and_getServiceId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD];

    SomeIpMessage message(buffer);
    message.setMessageId(0x0);
    EXPECT_EQ(static_cast<uint32_t>(0x0), message.getServiceId());
    EXPECT_EQ(static_cast<uint32_t>(0x0), message.getMethodId());
    message.setMessageId(0xFFFF);
    EXPECT_EQ(static_cast<uint32_t>(0x0), message.getServiceId());
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getMethodId());
    message.setMessageId(0xFFFF0000);
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getServiceId());
    EXPECT_EQ(static_cast<uint32_t>(0x0), message.getMethodId());
    message.setMessageId(0xFFFFFFFF);
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getServiceId());
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getMethodId());
}

/**
 * Test setting SomeIpMessage length.
 */
TEST_F(SomeIpMessageTest, test_setLength)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD];
    SomeIpMessage message(buffer);
    message.setLength(0x0);
    EXPECT_EQ(static_cast<uint8_t>(0x0), buffer[SomeIpMessage::OFFSET_LENGTH + 0]);
    EXPECT_EQ(static_cast<uint8_t>(0x0), buffer[SomeIpMessage::OFFSET_LENGTH + 1]);
    EXPECT_EQ(static_cast<uint8_t>(0x0), buffer[SomeIpMessage::OFFSET_LENGTH + 2]);
    EXPECT_EQ(static_cast<uint8_t>(0x0), buffer[SomeIpMessage::OFFSET_LENGTH + 3]);
    message.setLength(0x12345678);
    EXPECT_EQ(static_cast<uint8_t>(0x12), buffer[SomeIpMessage::OFFSET_LENGTH + 0]);
    EXPECT_EQ(static_cast<uint8_t>(0x34), buffer[SomeIpMessage::OFFSET_LENGTH + 1]);
    EXPECT_EQ(static_cast<uint8_t>(0x56), buffer[SomeIpMessage::OFFSET_LENGTH + 2]);
    EXPECT_EQ(static_cast<uint8_t>(0x78), buffer[SomeIpMessage::OFFSET_LENGTH + 3]);
    EXPECT_EQ(static_cast<uint32_t>(0x12345678), message.getLength());
}

/**
 * Test getting SomeIpMessage length (without header).
 */
TEST_F(SomeIpMessageTest, test_getLength)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD];
    SomeIpMessage message(buffer);
    buffer[SomeIpMessage::OFFSET_LENGTH + 0] = 0x87;
    buffer[SomeIpMessage::OFFSET_LENGTH + 1] = 0x65;
    buffer[SomeIpMessage::OFFSET_LENGTH + 2] = 0x43;
    buffer[SomeIpMessage::OFFSET_LENGTH + 3] = 0x21;
    EXPECT_EQ(static_cast<uint32_t>(0x87654321), message.getLength());
}

/**
 * Test getting length of payload.
 */
TEST_F(SomeIpMessageTest, test_getPayloadLength)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD];

    SomeIpMessage message(buffer);
    buffer[SomeIpMessage::OFFSET_LENGTH + 0] = 0x00;
    buffer[SomeIpMessage::OFFSET_LENGTH + 1] = 0x00;
    buffer[SomeIpMessage::OFFSET_LENGTH + 2] = 0x00;
    buffer[SomeIpMessage::OFFSET_LENGTH + 3] = 0x08;
    EXPECT_EQ(static_cast<uint32_t>(0), message.getPayloadLength());
    buffer[SomeIpMessage::OFFSET_LENGTH + 0] = 0x00;
    buffer[SomeIpMessage::OFFSET_LENGTH + 1] = 0x00;
    buffer[SomeIpMessage::OFFSET_LENGTH + 2] = 0x00;
    buffer[SomeIpMessage::OFFSET_LENGTH + 3] = 0x10;
    EXPECT_EQ(static_cast<uint32_t>(8), message.getPayloadLength());
    buffer[SomeIpMessage::OFFSET_LENGTH + 0] = 0x00;
    buffer[SomeIpMessage::OFFSET_LENGTH + 1] = 0x00;
    buffer[SomeIpMessage::OFFSET_LENGTH + 2] = 0x00;
    buffer[SomeIpMessage::OFFSET_LENGTH + 3] = 0x07;
    EXPECT_EQ(static_cast<uint32_t>(0), message.getPayloadLength());
}

/**
 * Test setting payload length.
 */
TEST_F(SomeIpMessageTest, test_setPayloadLength)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD];

    SomeIpMessage message(buffer);
    message.setPayloadLength(0U);
    EXPECT_EQ(static_cast<uint8_t>(0x00), buffer[SomeIpMessage::OFFSET_LENGTH + 0]);
    EXPECT_EQ(static_cast<uint8_t>(0x00), buffer[SomeIpMessage::OFFSET_LENGTH + 1]);
    EXPECT_EQ(static_cast<uint8_t>(0x00), buffer[SomeIpMessage::OFFSET_LENGTH + 2]);
    EXPECT_EQ(static_cast<uint8_t>(0x08), buffer[SomeIpMessage::OFFSET_LENGTH + 3]);
    message.setPayloadLength(10U);
    EXPECT_EQ(static_cast<uint8_t>(0x00), buffer[SomeIpMessage::OFFSET_LENGTH + 0]);
    EXPECT_EQ(static_cast<uint8_t>(0x00), buffer[SomeIpMessage::OFFSET_LENGTH + 1]);
    EXPECT_EQ(static_cast<uint8_t>(0x00), buffer[SomeIpMessage::OFFSET_LENGTH + 2]);
    EXPECT_EQ(static_cast<uint8_t>(0x12), buffer[SomeIpMessage::OFFSET_LENGTH + 3]);
}

/**
 * Test getting requestId.
 */
TEST_F(SomeIpMessageTest, test_getRequestId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD];

    SomeIpMessage message(buffer);
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 0] = 0x87;
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 1] = 0x65;
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 2] = 0x43;
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 3] = 0x21;
    EXPECT_EQ(static_cast<uint32_t>(0x87654321), message.getRequestId());
}

struct ParameterizedSetRequestIdTest : public ParameterizedUInt32Test
{};

/**
 * Make sure different requestId values can be set correctly.
 */
TEST_P(ParameterizedSetRequestIdTest, test_setRequestId_different_ids)
{
    message.setRequestId(GetParam());
    EXPECT_EQ(static_cast<uint32_t>(GetParam()), message.getRequestId());
}

INSTANTIATE_TEST_SUITE_P(
    test_setRequestId_different_ids,
    ParameterizedSetRequestIdTest,
    ::testing::Values(
        uint32_t(0),
        uint32_t(1),
        uint32_t(0x7FFFF),
        uint32_t(0x7FFE),
        uint32_t(0xFFFF),
        uint32_t(0x10000),
        uint32_t(0xFFFFFFFF)));

/**
 * Test getting clientId.
 */
TEST_F(SomeIpMessageTest, test_getClientId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD]
        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};

    SomeIpMessage message(buffer);
    EXPECT_EQ(static_cast<uint32_t>(0x1223), message.getClientId());
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 0] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFF23), message.getClientId());
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 1] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getClientId());
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 2] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getClientId());
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 3] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getClientId());
}

/**
 * Test setting clientId.
 */
TEST_F(SomeIpMessageTest, test_setClientId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD]
        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};

    SomeIpMessage message(buffer);
    message.setClientId(0xFFFF);
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getClientId());
    EXPECT_EQ(static_cast<uint8_t>(0x34), buffer[SomeIpMessage::OFFSET_REQUEST_ID + 2]);
    EXPECT_EQ(static_cast<uint8_t>(0x45), buffer[SomeIpMessage::OFFSET_REQUEST_ID + 3]);
}

/**
 * Test getting sessionId.
 */
TEST_F(SomeIpMessageTest, test_getSessionId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD]
        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};

    SomeIpMessage message(buffer);
    EXPECT_EQ(static_cast<uint32_t>(0x3445), message.getSessionId());
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 0] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0x3445), message.getSessionId());
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 1] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0x3445), message.getSessionId());
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 2] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFF45), message.getSessionId());
    buffer[SomeIpMessage::OFFSET_REQUEST_ID + 3] = 0xFF;
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getSessionId());
}

/**
 * Test setting sessionId.
 */
TEST_F(SomeIpMessageTest, test_setSessionId)
{
    uint8_t buffer[SomeIpMessage::OFFSET_PAYLOAD]
        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};

    SomeIpMessage message(buffer);
    message.setSessionId(0xFFFF);
    EXPECT_EQ(static_cast<uint32_t>(0xFFFF), message.getSessionId());
    EXPECT_EQ(static_cast<uint8_t>(0x12), buffer[SomeIpMessage::OFFSET_REQUEST_ID + 0]);
    EXPECT_EQ(static_cast<uint8_t>(0x23), buffer[SomeIpMessage::OFFSET_REQUEST_ID + 1]);
}

/**
 * Test getting protocolVersion.
 */
TEST_F(SomeIpMessageTest, test_getProtocolVersion)
{
    // clang-format off
    uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};
    // clang-format on

    SomeIpMessage message(buffer);
    EXPECT_EQ(static_cast<uint8_t>(0x12), message.getProtocolVersion());
    buffer[SomeIpMessage::OFFSET_PROTOCOL_VERSION] = 0xFF;
    EXPECT_EQ(static_cast<uint8_t>(0xFF), message.getProtocolVersion());
}

/**
 * Test setting protocolVersion.
 */
TEST_F(SomeIpMessageTest, test_setProtocolVersion)
{
    // clang-format off
    uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};
    // clang-format on
    SomeIpMessage message(buffer);
    message.setProtocolVersion(0xFF);
    EXPECT_EQ(static_cast<uint8_t>(0xFF), message.getProtocolVersion());
    EXPECT_EQ(static_cast<uint8_t>(0xFF), buffer[SomeIpMessage::OFFSET_PROTOCOL_VERSION]);
    EXPECT_EQ(static_cast<uint8_t>(0x23), buffer[SomeIpMessage::OFFSET_INTERFACE_VERSION]);
    EXPECT_EQ(static_cast<uint8_t>(0x34), buffer[SomeIpMessage::OFFSET_MESSAGE_TYPE]);
    EXPECT_EQ(static_cast<uint8_t>(0x45), buffer[SomeIpMessage::OFFSET_RETURN_CODE]);
}

/**
 * Test getting interfaceVersion.
 */
TEST_F(SomeIpMessageTest, test_getInterfaceVersion)
{
    // clang-format off
    uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};
    // clang-format on

    SomeIpMessage message(buffer);
    EXPECT_EQ(static_cast<uint8_t>(0x23), message.getInterfaceVersion());
    buffer[SomeIpMessage::OFFSET_INTERFACE_VERSION] = 0xFF;
    EXPECT_EQ(static_cast<uint8_t>(0xFF), message.getInterfaceVersion());
}

/**
 * Test setting interfaceVersion.
 */
TEST_F(SomeIpMessageTest, test_setInterfaceVersion)
{
    // clang-format off
    uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};
    // clang-format on

    SomeIpMessage message(buffer);
    message.setInterfaceVersion(0xFF);
    EXPECT_EQ(static_cast<uint8_t>(0xFF), message.getInterfaceVersion());
    EXPECT_EQ(static_cast<uint8_t>(0x12), buffer[SomeIpMessage::OFFSET_PROTOCOL_VERSION]);
    EXPECT_EQ(static_cast<uint8_t>(0xFF), buffer[SomeIpMessage::OFFSET_INTERFACE_VERSION]);
    EXPECT_EQ(static_cast<uint8_t>(0x34), buffer[SomeIpMessage::OFFSET_MESSAGE_TYPE]);
    EXPECT_EQ(static_cast<uint8_t>(0x45), buffer[SomeIpMessage::OFFSET_RETURN_CODE]);
}

/**
 * Test getting messageType of SomeIpMessage.
 */
TEST_F(SomeIpMessageTest, test_getMessageType)
{
    // clang-format off
    uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};
    // clang-format on

    SomeIpMessage message(buffer);
    EXPECT_EQ(static_cast<SomeIpMessage::MessageType>(0x34), message.getMessageType());
    buffer[SomeIpMessage::OFFSET_MESSAGE_TYPE] = 0xFF;
    EXPECT_EQ(static_cast<SomeIpMessage::MessageType>(0xFF), message.getMessageType());
}

/**
 * Test setting messageType of SomeIpMessage.
 */
TEST_F(SomeIpMessageTest, test_setMessageType)
{
    // clang-format off
    uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};
    // clang-format on
    SomeIpMessage message(buffer);
    message.setMessageType(SomeIpMessage::MessageType::REQUEST_NO_RETURN);
    EXPECT_EQ(SomeIpMessage::MessageType::REQUEST_NO_RETURN, message.getMessageType());
    EXPECT_EQ(static_cast<uint8_t>(0x12), buffer[SomeIpMessage::OFFSET_PROTOCOL_VERSION]);
    EXPECT_EQ(static_cast<uint8_t>(0x23), buffer[SomeIpMessage::OFFSET_INTERFACE_VERSION]);
    EXPECT_EQ(static_cast<uint8_t>(0x01), buffer[SomeIpMessage::OFFSET_MESSAGE_TYPE]);
    EXPECT_EQ(static_cast<uint8_t>(0x45), buffer[SomeIpMessage::OFFSET_RETURN_CODE]);
}

/**
 * Test getting returnCode.
 */
TEST_F(SomeIpMessageTest, test_getReturnCode)
{
    // clang-format off
    uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};
    // clang-format on

    SomeIpMessage message(buffer);
    EXPECT_EQ(static_cast<SomeIpMessage::ReturnCode>(0x45), message.getReturnCode());
    buffer[SomeIpMessage::OFFSET_RETURN_CODE] = 0xFF;
    EXPECT_EQ(static_cast<SomeIpMessage::ReturnCode>(0xFF), message.getReturnCode());
}

/**
 * Test setting returnCode.
 */
TEST_F(SomeIpMessageTest, test_setReturnCode)
{
    // clang-format off
    uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x12, 0x23, 0x34, 0x45};
    // clang-format on
    SomeIpMessage message(buffer);
    message.setReturnCode(SomeIpMessage::ReturnCode::SOMEIP_E_OK);
    EXPECT_EQ(SomeIpMessage::ReturnCode::SOMEIP_E_OK, message.getReturnCode());
    EXPECT_EQ(static_cast<uint8_t>(0x12), buffer[SomeIpMessage::OFFSET_PROTOCOL_VERSION]);
    EXPECT_EQ(static_cast<uint8_t>(0x23), buffer[SomeIpMessage::OFFSET_INTERFACE_VERSION]);
    EXPECT_EQ(static_cast<uint8_t>(0x34), buffer[SomeIpMessage::OFFSET_MESSAGE_TYPE]);
    EXPECT_EQ(
        static_cast<uint8_t>(SomeIpMessage::ReturnCode::SOMEIP_E_OK),
        buffer[SomeIpMessage::OFFSET_RETURN_CODE]);
}

/**
 * Make sure getting client to server Magic Cookie works correctly.
 */
TEST_F(SomeIpMessageTest, test_makeClientToServerMagicCookieMessage)
{
    uint8_t buffer[SomeIpConstants::HEADER_LENGTH];

    SomeIpMessage msg(buffer);
    SomeIpMessage::makeClientToServerMagicCookieMessage(msg);

    EXPECT_EQ(0xFFFFU, msg.getServiceId());
    EXPECT_EQ(0x0U, msg.getMethodId());
    EXPECT_EQ(0x8U, msg.getLength());
    EXPECT_EQ(0xDEADBEEFU, msg.getRequestId());
    EXPECT_EQ(0x1U, msg.getProtocolVersion());
    EXPECT_EQ(0x1U, msg.getInterfaceVersion());
    EXPECT_EQ(static_cast<SomeIpMessage::MessageType>(0x1U), msg.getMessageType());
    EXPECT_EQ(static_cast<SomeIpMessage::ReturnCode>(0x0U), msg.getReturnCode());
}

/**
 * Make sure getting server to client Magic Cookie works correctly.
 */
TEST_F(SomeIpMessageTest, test_makeServerToClientMagicCookieMessage)
{
    uint8_t buffer[SomeIpConstants::HEADER_LENGTH];

    SomeIpMessage msg(buffer);
    SomeIpMessage::makeServerToClientMagicCookieMessage(msg);

    EXPECT_EQ(0xFFFFU, msg.getServiceId());
    EXPECT_EQ(0x8000U, msg.getMethodId());
    EXPECT_EQ(0x8U, msg.getLength());
    EXPECT_EQ(0xDEADBEEFU, msg.getRequestId());
    EXPECT_EQ(0x1U, msg.getProtocolVersion());
    EXPECT_EQ(0x1U, msg.getInterfaceVersion());
    EXPECT_EQ(static_cast<SomeIpMessage::MessageType>(0x2U), msg.getMessageType());
    EXPECT_EQ(static_cast<SomeIpMessage::ReturnCode>(0x0U), msg.getReturnCode());
}

} // anonymous namespace
