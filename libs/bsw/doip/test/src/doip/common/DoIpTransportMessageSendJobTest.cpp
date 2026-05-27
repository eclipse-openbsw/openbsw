/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpTransportMessageSendJob.h"

#include "doip/common/DoIpHeader.h"
#include "doip/common/DoIpSendJobCallbackMock.h"
#include "doip/common/DoIpTestHelpers.h"

#include <etl/array.h>
#include <etl/span.h>
#include <transport/TransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

TEST(DoIpTransportMessageSendJobTest, TestConstructorWithAddressesFromMessage)
{
    uint8_t data[] = {
        0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87, 0x98, 0xa9, 0xba, 0xcb, 0xdc, 0xed, 0xfe};
    ::transport::TransportMessage message(data, sizeof(data));
    message.setPayloadLength(15U);
    message.setSourceAddress(0x1234U);
    message.setTargetAddress(0x4321U);
    ::transport::TransportMessageProcessedListenerMock processedListenerMock;
    DoIpSendJobCallbackMock<DoIpTransportMessageSendJob> sendJobCallbackMock;
    DoIpTransportMessageSendJob cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        message,
        &processedListenerMock,
        sendJobCallbackMock);
    EXPECT_EQ(&message, &cut.getMessage());
    EXPECT_EQ(&processedListenerMock, cut.getNotificationListener());
    EXPECT_EQ(3U, cut.getSendBufferCount());
    EXPECT_EQ(DoIpConstants::DOIP_HEADER_LENGTH + 4U + 15U, cut.getTotalLength());
    ::etl::array<uint8_t, 8U> staticBuffer;
    ::etl::span<uint8_t const> sendBuffer = cut.getSendBuffer(staticBuffer, 0U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    uint8_t const expectedHeader[] = {0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x13};
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(expectedHeader), sendBuffer));
    uint8_t const staticPayload[] = {0x12, 0x34, 0x43, 0x21};
    sendBuffer                    = cut.getSendBuffer(staticBuffer, 1U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(staticPayload), sendBuffer));
    sendBuffer = cut.getSendBuffer(staticBuffer, 2U);
    EXPECT_EQ(data, sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(data), sendBuffer));
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(), cut.getSendBuffer(staticBuffer, 3U)));
    // send processed without success
    EXPECT_CALL(
        processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ::transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR));
    cut.sendTransportMessageProcessed(false);
    Mock::VerifyAndClearExpectations(&processedListenerMock);
    // send processed with success
    EXPECT_CALL(
        processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ::transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    cut.sendTransportMessageProcessed(true);
    Mock::VerifyAndClearExpectations(&processedListenerMock);
    // release without success
    EXPECT_CALL(sendJobCallbackMock, releaseSendJob(Ref(cut), false));
    cut.release(false);
    // release with success
    EXPECT_CALL(sendJobCallbackMock, releaseSendJob(Ref(cut), true));
    cut.release(true);
}

TEST(DoIpTransportMessageSendJobTest, TestConstructorWithExplicitAddresses)
{
    uint8_t data[] = {
        0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87, 0x98, 0xa9, 0xba, 0xcb, 0xdc, 0xed, 0xfe};
    ::transport::TransportMessage message(data, sizeof(data));
    message.setPayloadLength(15U);
    message.setSourceAddress(0x1234U);
    message.setTargetAddress(0x4321U);
    ::transport::TransportMessageProcessedListenerMock processedListenerMock;
    DoIpSendJobCallbackMock<DoIpTransportMessageSendJob> sendJobCallbackMock;
    DoIpTransportMessageSendJob cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        message,
        &processedListenerMock,
        0x7753U,
        0x9813U,
        sendJobCallbackMock);
    EXPECT_EQ(0x7753U, cut.getSourceAddress());
    EXPECT_EQ(0x9813U, cut.getTargetAddress());
    EXPECT_EQ(&message, &cut.getMessage());
    EXPECT_EQ(&processedListenerMock, cut.getNotificationListener());
    EXPECT_EQ(3U, cut.getSendBufferCount());
    EXPECT_EQ(DoIpConstants::DOIP_HEADER_LENGTH + 4U + 15U, cut.getTotalLength());
    EXPECT_EQ(nullptr, cut.getDestinationEndpoint());
    ::etl::array<uint8_t, 8U> staticBuffer;
    ::etl::span<uint8_t const> sendBuffer = cut.getSendBuffer(staticBuffer, 0U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    uint8_t const expectedHeader[] = {0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x13};
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(expectedHeader), sendBuffer));
    uint8_t const staticPayload[] = {0x77, 0x53, 0x98, 0x13};
    sendBuffer                    = cut.getSendBuffer(staticBuffer, 1U);
    EXPECT_EQ(staticBuffer.data(), sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(staticPayload), sendBuffer));
    sendBuffer = cut.getSendBuffer(staticBuffer, 2U);
    EXPECT_EQ(data, sendBuffer.data());
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(data), sendBuffer));
    EXPECT_TRUE(is_equal(::etl::span<uint8_t const>(), cut.getSendBuffer(staticBuffer, 3U)));
    // send processed without success
    EXPECT_CALL(
        processedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ::transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR));
    cut.sendTransportMessageProcessed(false);
    Mock::VerifyAndClearExpectations(&processedListenerMock);
    // release with success
    EXPECT_CALL(sendJobCallbackMock, releaseSendJob(Ref(cut), true));
    cut.release(true);
}

TEST(DoIpTransportMessageSendJobTest, TestSendTransportMessageProcessedWithoutNotificationListener)
{
    uint8_t data[15] = {0x10};
    ::transport::TransportMessage message(data, sizeof(data));
    message.setPayloadLength(15U);
    message.setSourceAddress(0x1234U);
    message.setTargetAddress(0x4321U);
    DoIpSendJobCallbackMock<DoIpTransportMessageSendJob> sendJobCallbackMock;
    DoIpTransportMessageSendJob cut(
        DoIpConstants::ProtocolVersion::version02Iso2012, message, nullptr, sendJobCallbackMock);
    cut.sendTransportMessageProcessed(false);
}

} // namespace test
} // namespace doip
