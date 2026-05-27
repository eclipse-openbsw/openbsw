/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/server/DoIpServerTransportMessageHandler.h"

#include "doip/common/DoIpTestHelpers.h"
#include "doip/common/DoIpTransportMessageProvidingListenerHelper.h"
#include "doip/common/DoIpTransportMessageProvidingListenerMock.h"
#include "doip/server/DoIpServerConnectionMock.h"
#include "doip/server/DoIpServerTransportConnectionConfig.h"
#include "doip/server/DoIpServerTransportLayerParameters.h"

#include <common/busid/BusId.h>
#include <etl/pool.h>
#include <etl/unaligned_type.h>
#include <transport/BufferedTransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>

#include <gmock/gmock.h>
#include <gtest/esr_extensions.h>

using namespace ::testing;
using namespace ::transport;
using namespace ::doip;
using namespace ::doip::test;

namespace
{
DoIpHeader const& as_header(::etl::span<uint8_t const> bytes)
{
    return bytes.reinterpret_as<DoIpHeader const>()[0];
}
} // namespace

struct DoIpServerTransportMessageHandlerTest : Test
{
    DoIpServerTransportMessageHandlerTest()
    : fBusId(0x0)
    , asyncContext()
    , fParams(100, 200, 300, DOIP_MAX_PAYLOAD_LENGTH)
    , fConfig(
          fBusId,
          0x1234U,
          asyncContext,
          fMessageProvidingListenerMock,
          fMessageProcessedListenerMock,
          fParams)
    {}

    void checkGetTransportMessageNack(
        DoIpServerTransportMessageHandler& cut,
        ITransportMessageProvider::ErrorCode errorCode,
        uint16_t sourceAddress,
        uint16_t internalSourceAddress,
        uint8_t expectedNack,
        bool expectGetTransportMessage = true);
    void forceDiagnosticNack(
        DoIpServerTransportMessageHandler& cut, bool expectSendMessage, bool sendMessageResult);

    void SetUp()
    {
        EXPECT_CALL(fServerConnectionMock, resumeSending()).Times(AnyNumber());
        EXPECT_CALL(fServerConnectionMock, suspendSending()).Times(AnyNumber());
    }

    uint8_t fBusId;
    StrictMock<DoIpServerConnectionMock> fServerConnectionMock;
    StrictMock<DoIpTransportMessageProvidingListenerMock> fMessageProvidingListenerMock;
    StrictMock<TransportMessageProcessedListenerMock> fMessageProcessedListenerMock;
    ::async::ContextType asyncContext;
    DoIpServerTransportLayerParameters fParams;
    DoIpServerTransportConnectionConfig fConfig;
    ::etl::pool<DoIpTransportMessageSendJob, 4> fDiagnosticSendJobBlockPool;
    ::etl::pool<DoIpServerTransportMessageHandler::StaticPayloadSendJobType, 4>
        fProtocolSendJobBlockPool;
    uint8_t fHeaderBuffer[8U];
    static constexpr uint32_t DOIP_MAX_PAYLOAD_LENGTH
        = (16U * 1024U) - DoIpConstants::DOIP_HEADER_LENGTH;
};

TEST_F(
    DoIpServerTransportMessageHandlerTest,
    TestTransportMessageIsAllocatedWhenDiagnosticMessageIsReceived)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    // and active routing
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1234U));
    EXPECT_CALL(fServerConnectionMock, getInternalSourceAddress()).WillRepeatedly(Return(0x1357U));
    cut.routingActive();
    // check for diagnostic message header
    uint8_t const diagnosticMessage[] = {
        0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x07, 0x7e, 0x11, 0x22, 0x33};
    ::etl::span<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    EXPECT_TRUE(cut.headerReceived(as_header(diagnosticMessage)));
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    EXPECT_EQ(4U, payloadBuffer.size());
    // Expect allocation of message
    BufferedTransportMessage<3> message;
    EXPECT_CALL(
        fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1357, 0x077e, 0x03, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&message),
            Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                ITransportMessageProvider::ErrorCode::TPMSG_OK))));
    payloadCallback(::etl::span<uint8_t const>(diagnosticMessage).subspan(8U, 4U));
    EXPECT_EQ(3U, payloadBuffer.size());
    // Expect reception of payload
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_));
    EXPECT_CALL(fServerConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    EXPECT_CALL(
        fMessageProvidingListenerMock,
        messageReceived(fBusId, Ref(message), &fMessageProcessedListenerMock))
        .WillOnce(Return(DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
            ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
    payloadCallback(::etl::span<uint8_t const>(diagnosticMessage).subspan(12, 3U));
    auto const tpmBuffer = ::etl::span<uint8_t const>(message.getBuffer(), 3U);
    auto const diagMsg   = ::etl::span<uint8_t const>(diagnosticMessage).subspan(12U);
    // check that the 3-byte peek is correctly copied into the transport message buffer
    EXPECT_TRUE(is_equal(tpmBuffer, diagMsg));
    // Expect diagnostic ack
    uint8_t const expectedDiagnosticAck[] = {
        0x02,
        0xfd,
        0x80,
        0x02,
        0x00,
        0x00,
        0x00,
        0x08,
        0x07,
        0x7e,
        0x12,
        0x34,
        0x00,
        0x11,
        0x22,
        0x33,
    };
    EXPECT_EQ(2, sendJob->getSendBufferCount());
    EXPECT_THAT(
        sendJob->getSendBuffer(fHeaderBuffer, 0U),
        ::testing::ElementsAreArray(expectedDiagnosticAck, 8U));
    EXPECT_THAT(
        sendJob->getSendBuffer(fHeaderBuffer, 1U),
        ::testing::ElementsAreArray(expectedDiagnosticAck + 8U, 8U));
    // Release the send job and close the connection
    cut.connectionClosed();
    sendJob->release(false);
}

TEST_F(
    DoIpServerTransportMessageHandlerTest,
    TestTransportMessageDiagnosticMessageWithFullPeekIsReceived)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    // and active routing
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1234U));
    EXPECT_CALL(fServerConnectionMock, getInternalSourceAddress()).WillRepeatedly(Return(0x1357U));
    cut.routingActive();

    {
        // 5-byte diagnostic message that requires peek read and no additional remaining payload
        // read
        uint8_t const diagnosticMessage[]
            = {0x02,
               0xfd,
               0x80,
               0x01,
               0x00,
               0x00,
               0x00,
               0x09,
               0x12,
               0x34,
               0x07,
               0x7e,
               0x11,
               0x22,
               0x33,
               0x44,
               0x55};
        auto receiveSpan = ::etl::span<uint8_t const>(diagnosticMessage);

        // receive header
        ::etl::span<uint8_t> logicalAddressInfoReceiveBuffer;
        IDoIpConnection::PayloadReceivedCallbackType logicalAddressInfoReceiveCallback;
        EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
            .WillOnce(DoAll(
                SaveArg<0>(&logicalAddressInfoReceiveBuffer),
                SaveArg<1>(&logicalAddressInfoReceiveCallback),
                Return(true)));
        EXPECT_TRUE(cut.headerReceived(as_header(diagnosticMessage)));
        receiveSpan.advance(8U);
        EXPECT_EQ(4U, logicalAddressInfoReceiveBuffer.size());

        // receive logical address info
        ::etl::span<uint8_t> payloadPrefixReceiveBuffer;
        IDoIpConnection::PayloadReceivedCallbackType payloadPrefixReceiveCallback;
        EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
            .WillOnce(DoAll(
                SaveArg<0>(&payloadPrefixReceiveBuffer),
                SaveArg<1>(&payloadPrefixReceiveCallback),
                Return(true)));
        logicalAddressInfoReceiveCallback(receiveSpan.subspan(0U, 4U));
        receiveSpan.advance(4U);
        EXPECT_EQ(5U, payloadPrefixReceiveBuffer.size());

        // receive payload prefix and expect allocation of message
        BufferedTransportMessage<5U> message;
        ::etl::span<uint8_t const> peekSpan;
        EXPECT_CALL(
            fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1357, 0x077e, 0x05, _, _))
            .WillOnce(DoAll(
                SaveArg<4>(&peekSpan),
                SetArgReferee<5>(&message),
                Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                    ITransportMessageProvider::ErrorCode::TPMSG_OK))));
        IDoIpSendJob* sendJob = nullptr;
        EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_));
        EXPECT_CALL(fServerConnectionMock, sendMessage(_))
            .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
        EXPECT_CALL(
            fMessageProvidingListenerMock,
            messageReceived(fBusId, Ref(message), &fMessageProcessedListenerMock))
            .WillOnce(Return(DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
                ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
        payloadPrefixReceiveCallback(receiveSpan.subspan(0U, 5U));
        // check that the 4 bytes of the passed peek is correct
        EXPECT_THAT(peekSpan, ::testing::ElementsAreArray(receiveSpan.subspan(0U, 4U)));
        // check that the 5-byte prefix is correctly copied into the transport message buffer
        auto const tpmBuffer      = ::etl::span<uint8_t const>(message.getBuffer(), 5U);
        auto const diagMsgPayload = ::etl::span<uint8_t const>(diagnosticMessage).subspan(12U);
        EXPECT_THAT(tpmBuffer, ::testing::ElementsAreArray(diagMsgPayload));

        // Expect diagnostic ack
        uint8_t const expectedDiagnosticAck[] = {
            0x02,
            0xfd,
            0x80,
            0x02,
            0x00,
            0x00,
            0x00,
            0x0A,
            0x07,
            0x7e,
            0x12,
            0x34,
            0x00,
            0x11,
            0x22,
            0x33,
            0x44,
            0x55,
        };
        EXPECT_EQ(2, sendJob->getSendBufferCount());
        EXPECT_THAT(
            sendJob->getSendBuffer(fHeaderBuffer, 0U),
            ::testing::ElementsAreArray(expectedDiagnosticAck, 8U));
        EXPECT_THAT(
            sendJob->getSendBuffer(fHeaderBuffer, 1U),
            ::testing::ElementsAreArray(expectedDiagnosticAck + 8U, 10U));
        sendJob->release(false);
    }

    {
        // six byte diagnostic message that requires peek read and then remaining payload read
        uint8_t const diagnosticMessage[]
            = {0x02,
               0xfd,
               0x80,
               0x01,
               0x00,
               0x00,
               0x00,
               0x0a,
               0x12,
               0x34,
               0x07,
               0x7e,
               0x11,
               0x22,
               0x33,
               0x44,
               0x55,
               0x66};
        auto receiveSpan = etl::span<uint8_t const>(diagnosticMessage);

        // receive header
        ::etl::span<uint8_t> logicalAddressInfoReceiveBuffer;
        IDoIpConnection::PayloadReceivedCallbackType logicalAddressInfoReceiveCallback;
        EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
            .WillOnce(DoAll(
                SaveArg<0>(&logicalAddressInfoReceiveBuffer),
                SaveArg<1>(&logicalAddressInfoReceiveCallback),
                Return(true)));
        EXPECT_TRUE(cut.headerReceived(as_header(diagnosticMessage)));
        receiveSpan.advance(8U);
        EXPECT_EQ(4U, logicalAddressInfoReceiveBuffer.size());

        // receive logical address info
        ::etl::span<uint8_t> payloadPrefixReceiveBuffer;
        IDoIpConnection::PayloadReceivedCallbackType payloadPrefixReceiveCallback;
        EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
            .WillOnce(DoAll(
                SaveArg<0>(&payloadPrefixReceiveBuffer),
                SaveArg<1>(&payloadPrefixReceiveCallback),
                Return(true)));
        logicalAddressInfoReceiveCallback(receiveSpan.subspan(0U, 4U));
        receiveSpan.advance(4U);
        EXPECT_EQ(5U, payloadPrefixReceiveBuffer.size());

        // receive payload prefix
        ::etl::span<uint8_t> diagnosticMessageUserDataReceiveBuffer;
        IDoIpConnection::PayloadReceivedCallbackType diagnosticMessageUserDataReceiveCallback;
        EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
            .WillOnce(DoAll(
                SaveArg<0>(&diagnosticMessageUserDataReceiveBuffer),
                SaveArg<1>(&diagnosticMessageUserDataReceiveCallback),
                Return(true)));
        // Expect allocation of message
        BufferedTransportMessage<6U> message;
        ::etl::span<uint8_t const> peekSpan;
        EXPECT_CALL(
            fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1357, 0x077e, 0x06, _, _))
            .WillOnce(DoAll(
                SaveArg<4>(&peekSpan),
                SetArgReferee<5>(&message),
                Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                    ITransportMessageProvider::ErrorCode::TPMSG_OK))));
        payloadPrefixReceiveCallback(receiveSpan.subspan(0U, 5U));
        // check that the 4 bytes of the passed peek is correct
        EXPECT_THAT(peekSpan, ::testing::ElementsAreArray(receiveSpan.subspan(0U, 4U)));
        receiveSpan.advance(5U);

        // receive rest of payload
        IDoIpSendJob* sendJob = nullptr;
        EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_));
        EXPECT_CALL(fServerConnectionMock, sendMessage(_))
            .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
        EXPECT_CALL(
            fMessageProvidingListenerMock,
            messageReceived(fBusId, Ref(message), &fMessageProcessedListenerMock))
            .WillOnce(Return(DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
                ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));
        // the remaining byte is copied into the buffer of the receivePayload call and the callback
        // is called
        ::etl::mem_copy(
            receiveSpan.begin(),
            receiveSpan.size(),
            diagnosticMessageUserDataReceiveBuffer.begin());
        diagnosticMessageUserDataReceiveCallback(receiveSpan.subspan(0U, 1U));
        // check that the 5-byte prefix is correctly copied into the transport message buffer
        auto const tpmBuffer      = ::etl::span<uint8_t const>(message.getBuffer(), 6U);
        auto const diagMsgPayload = ::etl::span<uint8_t const>(diagnosticMessage).subspan(12U);
        EXPECT_THAT(tpmBuffer, ::testing::ElementsAreArray(diagMsgPayload));

        // Expect diagnostic ack
        uint8_t const expectedDiagnosticAck[] = {
            0x02,
            0xfd,
            0x80,
            0x02,
            0x00,
            0x00,
            0x00,
            0x0A,
            0x07,
            0x7e,
            0x12,
            0x34,
            0x00,
            0x11,
            0x22,
            0x33,
            0x44,
            0x55,
        };
        EXPECT_EQ(2, sendJob->getSendBufferCount());
        EXPECT_THAT(
            sendJob->getSendBuffer(fHeaderBuffer, 0U),
            ::testing::ElementsAreArray(expectedDiagnosticAck, 8U));
        EXPECT_THAT(
            sendJob->getSendBuffer(fHeaderBuffer, 1U),
            ::testing::ElementsAreArray(expectedDiagnosticAck + 8U, 10U));

        sendJob->release(false);
    }

    // Release the send job and close the connection
    cut.connectionClosed();
}

TEST_F(DoIpServerTransportMessageHandlerTest, TestUnprocessedTransportMessageCausesDiagnosticNack)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    // and active routing
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1234U));
    EXPECT_CALL(fServerConnectionMock, getInternalSourceAddress()).WillRepeatedly(Return(0x1357U));
    // and active routing
    cut.routingActive();
    // check for diagnostic message header
    uint8_t const diagnosticMessage[] = {
        0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x07, 0x7e, 0x11, 0x22, 0x33};
    ::etl::span<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    EXPECT_TRUE(cut.headerReceived(as_header(diagnosticMessage)));
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    EXPECT_EQ(4U, payloadBuffer.size());
    // Expect allocation of message
    BufferedTransportMessage<3> message;
    EXPECT_CALL(
        fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1357, 0x077e, 0x03, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&message),
            Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                ITransportMessageProvider::ErrorCode::TPMSG_OK))));
    payloadCallback(::etl::span<uint8_t const>(diagnosticMessage).subspan(8U, 4U));
    EXPECT_EQ(3U, payloadBuffer.size());
    // Expect reception of payload
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_)).Times(1);
    EXPECT_CALL(fServerConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    EXPECT_CALL(
        fMessageProvidingListenerMock,
        messageReceived(fBusId, Ref(message), &fMessageProcessedListenerMock))
        .WillOnce(Return(DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
            ITransportMessageListener::ReceiveResult::RECEIVED_ERROR)));
    EXPECT_CALL(fMessageProvidingListenerMock, releaseTransportMessage(Ref(message)));
    payloadCallback(::etl::span<uint8_t const>(diagnosticMessage).subspan(12U, 3U));
    auto const tpmBuffer      = ::etl::span<uint8_t const>(message.getBuffer(), 3U);
    // payload of diag message starts after 8 bytes of header and 4 bytes of src+dst addresses
    auto const diagMsgPayload = ::etl::span<uint8_t const>(diagnosticMessage).subspan(12U);
    // check that the 3-byte peek is correctly copied into the transport message buffer
    EXPECT_TRUE(is_equal(tpmBuffer, diagMsgPayload));
    // Expect diagnostic nack
    uint8_t const expectedDiagnosticNack[] = {
        0x02,
        0xfd,
        0x80,
        0x03,
        0x00,
        0x00,
        0x00,
        0x08,
        0x07,
        0x7e,
        0x12,
        0x34,
        DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TRANSPORT_PROTOCOL_ERROR,
        0x11,
        0x22,
        0x33,
    };
    EXPECT_EQ(2, sendJob->getSendBufferCount());
    EXPECT_THAT(
        sendJob->getSendBuffer(fHeaderBuffer, 0U),
        ::testing::ElementsAreArray(expectedDiagnosticNack, 8U));
    EXPECT_THAT(
        sendJob->getSendBuffer(fHeaderBuffer, 1U),
        ::testing::ElementsAreArray(expectedDiagnosticNack + 8U, 8U));
    // Release the send job and close the connection
    cut.connectionClosed();
    sendJob->release(false);
}

TEST_F(
    DoIpServerTransportMessageHandlerTest,
    TestDiagnosticMessageWithInvalidPayloadLengthsCausesNacks)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);

    // too long diagnostic message header
    {
        ::etl::be_uint32_t const payloadLen = DOIP_MAX_PAYLOAD_LENGTH + 1;
        uint8_t const diagnosticMessage[]
            = {0x02, 0xfd, 0x80, 0x01, payloadLen[0], payloadLen[1], payloadLen[2], payloadLen[3]};
        EXPECT_CALL(
            fServerConnectionMock,
            sendNack(DoIpConstants::NackCodes::NACK_MESSAGE_TOO_LARGE, false));
        EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_));
        EXPECT_TRUE(cut.headerReceived(as_header(diagnosticMessage)));
    }

    // too short diagnostic message header
    {
        uint8_t const diagnosticMessage[] = {0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x03};
        EXPECT_CALL(
            fServerConnectionMock,
            sendNack(DoIpConstants::NackCodes::NACK_INVALID_PAYLOAD_LENGTH, true));
        EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_));
        EXPECT_TRUE(cut.headerReceived(as_header(diagnosticMessage)));
    }
}

TEST_F(DoIpServerTransportMessageHandlerTest, TestGetTransportMessageErrorsCausesDiagnosticNacks)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_)).Times(4);
    // and active routing
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1122U));
    EXPECT_CALL(fServerConnectionMock, getInternalSourceAddress()).WillRepeatedly(Return(0x1357U));
    cut.routingActive();
    // expect diagnostic nacks on:
    checkGetTransportMessageNack(
        cut,
        ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS,
        0x1122U,
        0x1357U,
        DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_TARGET_ADDRESS);
    checkGetTransportMessageNack(
        cut,
        ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE,
        0x1122U,
        0x1357U,
        DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_DIAGNOSTIC_MESSAGE_TOO_LARGE);
    checkGetTransportMessageNack(
        cut,
        ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE,
        0x1122U,
        0x1357U,
        DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_OUT_OF_MEMORY);
    checkGetTransportMessageNack(
        cut,
        ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE,
        0x1122U,
        0x1357U,
        DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_UNKNOWN_NETWORK);
}

TEST_F(
    DoIpServerTransportMessageHandlerTest,
    TestGetTransportMessageErrorsCausesDiagnosticNackAndClosesBeforeActivation)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1122U));
    EXPECT_CALL(fServerConnectionMock, getInternalSourceAddress()).WillRepeatedly(Return(0x1357U));
    EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_)).Times(1);
    // expect diagnostic nacks before activation
    EXPECT_CALL(fServerConnectionMock, close()).Times(1);
    checkGetTransportMessageNack(
        cut,
        ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ADDRESS,
        0x1122U,
        0x1357U,
        DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_SOURCE_ADDRESS,
        false);
}

TEST_F(
    DoIpServerTransportMessageHandlerTest,
    TestGetTransportMessageErrorsCausesDiagnosticNackOnWrongSourceAddress)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1122U));
    EXPECT_CALL(fServerConnectionMock, getInternalSourceAddress()).WillRepeatedly(Return(0x1357U));
    EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_)).Times(1);
    // expect diagnostic nacks before activation
    cut.routingActive();
    EXPECT_CALL(fServerConnectionMock, close()).Times(1);
    checkGetTransportMessageNack(
        cut,
        ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ADDRESS,
        0x1123U,
        0x1357U,
        DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_SOURCE_ADDRESS,
        false);
}

TEST_F(
    DoIpServerTransportMessageHandlerTest,
    TestGetTransportMessageErrorsCausesDiagnosticNackOnCorrectSourceAddress)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1122U));
    EXPECT_CALL(fServerConnectionMock, getInternalSourceAddress()).WillRepeatedly(Return(0x1357U));
    EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_)).Times(1);
    // expect diagnostic nacks before activation
    cut.routingActive();
    EXPECT_CALL(fServerConnectionMock, close()).Times(1);
    checkGetTransportMessageNack(
        cut,
        ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ADDRESS,
        0x1122U,
        0x1357U,
        DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_SOURCE_ADDRESS);
}

TEST_F(DoIpServerTransportMessageHandlerTest, TestUnknownMessageTypeIsNotHandled)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    // open the connection and start routing
    cut.connectionOpened(fServerConnectionMock);
    cut.routingActive(); // 0x1124);
    // send an arbitrary message type
    uint8_t const diagnosticMessage[] = {0x02, 0xfd, 0x80, 0x05, 0x00, 0x00, 0x00, 0x03};
    EXPECT_FALSE(cut.headerReceived(as_header(diagnosticMessage)));
}

TEST_F(DoIpServerTransportMessageHandlerTest, TestSendTransportMessageWhenConnectionIsOpened)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    BufferedTransportMessage<3U> message;
    message.setSourceAddress(0x1357U);
    message.setTargetAddress(0x77eU);
    uint8_t const payload[] = {0x11, 0x22, 0x33};
    message.append(payload, sizeof(payload));
    message.setPayloadLength(sizeof(payload));
    EXPECT_FALSE(cut.send(message, &fMessageProcessedListenerMock));
    // source addresses
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1234U));
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    // try to send the message
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fServerConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    EXPECT_TRUE(cut.send(message, &fMessageProcessedListenerMock));
    EXPECT_TRUE(sendJob != nullptr);
    uint8_t const diagnosticMessage[] = {
        0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07, 0x13, 0x57, 0x12, 0x34, 0x11, 0x22, 0x33};
    EXPECT_EQ(3, sendJob->getSendBufferCount());
    EXPECT_TRUE(is_equal(
        ::etl::span<uint8_t const>(diagnosticMessage, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(is_equal(
        ::etl::span<uint8_t const>(diagnosticMessage + 8U, 4U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    EXPECT_TRUE(is_equal(
        ::etl::span<uint8_t const>(diagnosticMessage + 12U, 3U),
        sendJob->getSendBuffer(fHeaderBuffer, 2U)));
    // expect release when send job is being released
    EXPECT_CALL(
        fMessageProcessedListenerMock,
        transportMessageProcessed(
            Ref(message),
            ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));
    sendJob->release(true);
}

TEST_F(DoIpServerTransportMessageHandlerTest, TestSendTransportMessageWithFailedSendJob)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    BufferedTransportMessage<3U> message;
    message.setSourceAddress(0x1357U);
    message.setTargetAddress(0x77eU);
    uint8_t const payload[] = {0x11, 0x22, 0x33};
    message.append(payload, sizeof(payload));
    message.setPayloadLength(sizeof(payload));
    EXPECT_FALSE(cut.send(message, &fMessageProcessedListenerMock));
    // source addresses
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1234U));
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    // try to send the message
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fServerConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    EXPECT_TRUE(cut.send(message, &fMessageProcessedListenerMock));
    EXPECT_TRUE(sendJob != nullptr);
    uint8_t const diagnosticMessage[] = {
        0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07, 0x13, 0x57, 0x12, 0x34, 0x11, 0x22, 0x33};
    EXPECT_EQ(3, sendJob->getSendBufferCount());
    EXPECT_TRUE(is_equal(
        ::etl::span<uint8_t const>(diagnosticMessage, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(is_equal(
        ::etl::span<uint8_t const>(diagnosticMessage + 8U, 4U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    EXPECT_TRUE(is_equal(
        ::etl::span<uint8_t const>(diagnosticMessage + 12U, 3U),
        sendJob->getSendBuffer(fHeaderBuffer, 2U)));
    // expect release when send job is being released
    EXPECT_CALL(
        fMessageProcessedListenerMock,
        transportMessageProcessed(
            Ref(message), ITransportMessageProcessedListener::ProcessingResult::PROCESSED_ERROR));
    sendJob->release(false);
}

TEST_F(DoIpServerTransportMessageHandlerTest, TestSendTransportMessageWithoutNotificationListener)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    BufferedTransportMessage<3U> message;
    message.setSourceAddress(0x1357U);
    message.setTargetAddress(0x77eU);
    uint8_t const payload[] = {0x11, 0x22, 0x33};
    message.append(payload, sizeof(payload));
    message.setPayloadLength(sizeof(payload));
    EXPECT_FALSE(cut.send(message, &fMessageProcessedListenerMock));
    // source addresses
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1234U));
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    // try to send the message
    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fServerConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    EXPECT_TRUE(cut.send(message, nullptr));
    EXPECT_TRUE(sendJob != nullptr);
    uint8_t const diagnosticMessage[] = {
        0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07, 0x13, 0x57, 0x12, 0x34, 0x11, 0x22, 0x33};
    EXPECT_EQ(3, sendJob->getSendBufferCount());
    EXPECT_TRUE(is_equal(
        ::etl::span<uint8_t const>(diagnosticMessage, 8U),
        sendJob->getSendBuffer(fHeaderBuffer, 0U)));
    EXPECT_TRUE(is_equal(
        ::etl::span<uint8_t const>(diagnosticMessage + 8U, 4U),
        sendJob->getSendBuffer(fHeaderBuffer, 1U)));
    EXPECT_TRUE(is_equal(
        ::etl::span<uint8_t const>(diagnosticMessage + 12U, 3U),
        sendJob->getSendBuffer(fHeaderBuffer, 2U)));
    // expect release when send job is being released
    sendJob->release(true);
}

TEST_F(
    DoIpServerTransportMessageHandlerTest, TestSendTransportMessageFailsWhenConnectionsCannotSend)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    BufferedTransportMessage<3U> message;
    message.setSourceAddress(0x1357U);
    message.setTargetAddress(0x77eU);
    // source addresses
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1234U));
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    // send too many transport messages
    EXPECT_CALL(fServerConnectionMock, sendMessage(_)).WillOnce(Return(false));
    EXPECT_FALSE(cut.send(message, &fMessageProcessedListenerMock));
}

TEST_F(DoIpServerTransportMessageHandlerTest, TestSendTransportMessageFailsWhenAllSendJobsAreInUse)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    BufferedTransportMessage<3U> message;
    message.setSourceAddress(0x1357U);
    message.setTargetAddress(0x77eU);
    // source addresses
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1234U));
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    // send too many transport messages
    EXPECT_CALL(fServerConnectionMock, sendMessage(_)).Times(4).WillRepeatedly(Return(true));
    EXPECT_TRUE(cut.send(message, &fMessageProcessedListenerMock));
    EXPECT_TRUE(cut.send(message, &fMessageProcessedListenerMock));
    EXPECT_TRUE(cut.send(message, &fMessageProcessedListenerMock));
    EXPECT_TRUE(cut.send(message, &fMessageProcessedListenerMock));
    EXPECT_FALSE(cut.send(message, &fMessageProcessedListenerMock));
}

TEST_F(DoIpServerTransportMessageHandlerTest, TestSendDiagnosticNackWithoutSendJobRelease)
{
    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        fProtocolSendJobBlockPool,
        fConfig);
    BufferedTransportMessage<3U> message;
    message.setSourceAddress(0x1234U);
    message.setTargetAddress(0x77eU);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_)).Times(6);
    // force first send message to fail
    forceDiagnosticNack(cut, true, false);
    // now four times until pool is empty
    forceDiagnosticNack(cut, true, true);
    forceDiagnosticNack(cut, true, true);
    forceDiagnosticNack(cut, true, true);
    forceDiagnosticNack(cut, true, true);
    // the last should fail
    forceDiagnosticNack(cut, false, false);
}

TEST_F(DoIpServerTransportMessageHandlerTest, TestNoMessageReceivedIfProtocolSendJobPoolDepleted)
{
    ::etl::pool<DoIpServerTransportMessageHandler::StaticPayloadSendJobType, 1>
        sizeOneProtocolSendJobBlockPool;

    DoIpServerTransportMessageHandler cut(
        DoIpConstants::ProtocolVersion::version02Iso2012,
        fDiagnosticSendJobBlockPool,
        sizeOneProtocolSendJobBlockPool,
        fConfig);
    // open the connection
    cut.connectionOpened(fServerConnectionMock);
    // and active routing
    EXPECT_CALL(fServerConnectionMock, getSourceAddress()).WillRepeatedly(Return(0x1234U));
    EXPECT_CALL(fServerConnectionMock, getInternalSourceAddress()).WillRepeatedly(Return(0x1357U));
    cut.routingActive();
    // check for diagnostic message header
    uint8_t const diagnosticMessage[] = {
        0x02, 0xfd, 0x80, 0x01, 0x00, 0x00, 0x00, 0x07, 0x12, 0x34, 0x07, 0x7e, 0x11, 0x22, 0x33};
    ::etl::span<uint8_t> payloadBuffer;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallback;
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    EXPECT_TRUE(cut.headerReceived(as_header(diagnosticMessage)));
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(SaveArg<0>(&payloadBuffer), SaveArg<1>(&payloadCallback), Return(true)));
    EXPECT_EQ(4U, payloadBuffer.size());
    // Expect allocation of message
    BufferedTransportMessage<3> message;
    EXPECT_CALL(
        fMessageProvidingListenerMock, getTransportMessage(fBusId, 0x1357, 0x077e, 0x03, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&message),
            Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(
                ITransportMessageProvider::ErrorCode::TPMSG_OK))));
    payloadCallback(::etl::span<uint8_t const>(diagnosticMessage).subspan(8U, 4U));
    EXPECT_EQ(3U, payloadBuffer.size());
    // Since the ACK pool has capacity 1, expect sendMessage to return false
    EXPECT_CALL(fServerConnectionMock, sendMessage(_)).WillOnce(Return(false));
    EXPECT_CALL(fMessageProvidingListenerMock, releaseTransportMessage(Ref(message)));
    EXPECT_CALL(fServerConnectionMock, endReceiveMessage(_));
    payloadCallback(::etl::span<uint8_t const>(diagnosticMessage).subspan(12U, 3U));
    auto const tpmBuffer      = ::etl::span<uint8_t const>(message.getBuffer(), 3U);
    // payload of diag message starts after 8 bytes of header and 4 bytes of src+dst addresses
    auto const diagMsgPayload = ::etl::span<uint8_t const>(diagnosticMessage).subspan(12U);
    // check that the 3-byte peek is correctly copied into the transport message buffer
    EXPECT_TRUE(is_equal(tpmBuffer, diagMsgPayload));
    // Release the send job and close the connection
    cut.connectionClosed();
}

void DoIpServerTransportMessageHandlerTest::checkGetTransportMessageNack(
    DoIpServerTransportMessageHandler& cut,
    ITransportMessageProvider::ErrorCode errorCode,
    uint16_t sourceAddress,
    uint16_t internalSourceAddress,
    uint8_t expectedNack,
    bool expectGetTransportMessage)
{
    uint8_t const diagnosticMessage[]
        = {0x02,
           0xfd,
           0x80,
           0x01,
           0x00,
           0x00,
           0x00,
           0x09,
           static_cast<uint8_t>((sourceAddress >> 8) & 0xff),
           static_cast<uint8_t>(sourceAddress & 0xff),
           0x33,
           0x44,
           0x12,
           0x34,
           0x45,
           0x56,
           0x67};
    ::etl::span<uint8_t> payloadBufferLogicalAddress;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallbackLogicalAddress;
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(
            SaveArg<0>(&payloadBufferLogicalAddress),
            SaveArg<1>(&payloadCallbackLogicalAddress),
            Return(true)));
    EXPECT_TRUE(cut.headerReceived(as_header(diagnosticMessage)));
    EXPECT_TRUE(payloadCallbackLogicalAddress);
    EXPECT_EQ(4U, payloadBufferLogicalAddress.size());
    if (expectGetTransportMessage)
    {
        EXPECT_CALL(
            fMessageProvidingListenerMock,
            getTransportMessage(fBusId, internalSourceAddress, 0x3344U, 5U, _, _))
            .WillOnce(
                Return(DoIpTransportMessageProvidingListenerHelper::createGetResult(errorCode)));
    }
    ::etl::span<uint8_t> payloadBufferUserData;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallbackUserData;
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(
            SaveArg<0>(&payloadBufferUserData),
            SaveArg<1>(&payloadCallbackUserData),
            Return(true)));
    auto diagMsg = ::etl::span<uint8_t const>(diagnosticMessage);
    diagMsg.advance(8U); // skip header
    payloadCallbackLogicalAddress(diagMsg.subspan(0U, 4U));
    EXPECT_TRUE(payloadCallbackUserData);
    // read the 5-byte data prefix for peek or nack
    EXPECT_EQ(5U, payloadBufferUserData.size());

    IDoIpSendJob* sendJob = nullptr;
    EXPECT_CALL(fServerConnectionMock, sendMessage(_))
        .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(true)));
    diagMsg.advance(4U); // skip src + dst addresses
    payloadCallbackUserData(diagMsg);
    EXPECT_EQ(2, sendJob->getSendBufferCount());

    uint8_t const expectedMessageHeader[] = {0x02, 0xfd, 0x80, 0x03, 0x00, 0x00, 0x00, 0x0a};
    EXPECT_THAT(
        expectedMessageHeader,
        ::testing::ElementsAreArray(sendJob->getSendBuffer(fHeaderBuffer, 0U).data(), 8U));

    uint8_t const expectedMessageBody[]
        = {0x33,
           0x44,
           static_cast<uint8_t>((sourceAddress >> 8) & 0xff),
           static_cast<uint8_t>(sourceAddress & 0xff),
           expectedNack,
           0x12,
           0x34,
           0x45,
           0x56,
           0x67};
    uint8_t actualMessageBody[10];
    EXPECT_THAT(
        expectedMessageBody,
        ::testing::ElementsAreArray(sendJob->getSendBuffer(actualMessageBody, 1U).data(), 10U));
    sendJob->release(false);
}

void DoIpServerTransportMessageHandlerTest::forceDiagnosticNack(
    DoIpServerTransportMessageHandler& cut, bool expectSendMessage, bool sendMessageResult)
{
    uint8_t const diagnosticMessage[]
        = {0x02,
           0xfd,
           0x80,
           0x01,
           0x00,
           0x00,
           0x00,
           0x09,
           0x11,
           0x22,
           0x33,
           0x44,
           0x01,
           0x12,
           0x23,
           0x34,
           0x45};
    ::etl::span<uint8_t> payloadBufferLogicalAddress;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallbackLogicalAddress;
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(
            SaveArg<0>(&payloadBufferLogicalAddress),
            SaveArg<1>(&payloadCallbackLogicalAddress),
            Return(true)));
    EXPECT_TRUE(cut.headerReceived(as_header(diagnosticMessage)));
    EXPECT_TRUE(payloadCallbackLogicalAddress);
    EXPECT_EQ(4U, payloadBufferLogicalAddress.size());

    ::etl::span<uint8_t> payloadBufferUserData;
    IDoIpConnection::PayloadReceivedCallbackType payloadCallbackUserData;
    EXPECT_CALL(fServerConnectionMock, receivePayload(_, _))
        .WillOnce(DoAll(
            SaveArg<0>(&payloadBufferUserData),
            SaveArg<1>(&payloadCallbackUserData),
            Return(true)));
    payloadCallbackLogicalAddress(::etl::span<uint8_t const>(diagnosticMessage + 8U, 4U));
    EXPECT_TRUE(payloadCallbackUserData);
    EXPECT_EQ(5U, payloadBufferUserData.size());

    IDoIpSendJob* sendJob = nullptr;
    if (expectSendMessage)
    {
        EXPECT_CALL(fServerConnectionMock, sendMessage(_))
            .WillOnce(DoAll(SaveRef<0>(&sendJob), Return(sendMessageResult)));
    }
    payloadCallbackUserData(::etl::span<uint8_t const>(diagnosticMessage + 12U, 5U));
}
