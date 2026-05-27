/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpTransportMessageProvidingListenerHelper.h"

#include "doip/common/DoIpConstants.h"
#include "doip/common/DoIpTransportMessageProvidingListenerMock.h"

#include <transport/TransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>
#include <transport/TransportMessageProvidingListenerMock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

class DoIpTransportMessageProvidingListenerHelperTest : public Test
{
public:
    DoIpTransportMessageProvidingListenerHelperTest() : fFallbackProvidingListenerMock(false) {}

protected:
    DoIpTransportMessageProvidingListenerMock fProvidingListenerMock;
    ::transport::TransportMessage fTransportMessage;
    ::transport::TransportMessageProvidingListenerMock fFallbackProvidingListenerMock;
    ::transport::TransportMessageProcessedListenerMock fTransportProcessedListenerMock;
};

TEST_F(DoIpTransportMessageProvidingListenerHelperTest, TestForwarding)
{
    DoIpTransportMessageProvidingListenerHelper cut(fFallbackProvidingListenerMock);
    cut.setTransportMessageProvidingListener(&fProvidingListenerMock);
    // getTransportMessage
    ::transport::TransportMessage* transportMessage = nullptr;
    EXPECT_CALL(fProvidingListenerMock, getTransportMessage(0U, 123U, 234U, 100U, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&fTransportMessage),
            Return(IDoIpTransportMessageProvidingListener::GetResult(
                ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE,
                DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TARGET_UNREACHABLE))));
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TARGET_UNREACHABLE),
        cut.getTransportMessage(0U, 123U, 234U, 100U, {}, transportMessage));
    EXPECT_EQ(&fTransportMessage, transportMessage);
    // release TransportMessage
    EXPECT_CALL(fProvidingListenerMock, releaseTransportMessage(Ref(fTransportMessage)));
    cut.releaseTransportMessage(fTransportMessage);
    // messageReceived
    EXPECT_CALL(
        fProvidingListenerMock,
        messageReceived(0U, Ref(fTransportMessage), &fTransportProcessedListenerMock))
        .WillOnce(Return(IDoIpTransportMessageProvidingListener::ReceiveResult(
            ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_TARGET_ADDRESS)));
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::ReceiveResult(
            ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_TARGET_ADDRESS),
        cut.messageReceived(0U, fTransportMessage, &fTransportProcessedListenerMock));
}

TEST_F(DoIpTransportMessageProvidingListenerHelperTest, TestFallback)
{
    DoIpTransportMessageProvidingListenerHelper cut(fFallbackProvidingListenerMock);
    // getTransportMessage
    ::transport::TransportMessage* transportMessage = nullptr;
    EXPECT_CALL(fFallbackProvidingListenerMock, getTransportMessage(0U, 123U, 234U, 100U, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&fTransportMessage),
            Return(::transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE)));
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_OUT_OF_MEMORY),
        cut.getTransportMessage(0U, 123U, 234U, 100U, {}, transportMessage));
    // release TransportMessage
    EXPECT_CALL(fFallbackProvidingListenerMock, releaseTransportMessage(Ref(fTransportMessage)));
    cut.releaseTransportMessage(fTransportMessage);
    // messageReceived
    EXPECT_CALL(
        fFallbackProvidingListenerMock,
        messageReceived(0U, Ref(fTransportMessage), &fTransportProcessedListenerMock))
        .WillOnce(Return(::transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR));
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::ReceiveResult(
            ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TRANSPORT_PROTOCOL_ERROR),
        cut.messageReceived(0U, fTransportMessage, &fTransportProcessedListenerMock));
}

TEST_F(DoIpTransportMessageProvidingListenerHelperTest, TestCreateGetResultFromErrorCode)
{
    // TPMSG_OK
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_OK,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_UNKNOWN_NETWORK),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_OK));
    // TPMSG_INVALID_SRC_ADDRESS,
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ADDRESS,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_SOURCE_ADDRESS),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ADDRESS));
    // TPMSG_INVALID_TGT_ADDRESS,
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_TARGET_ADDRESS),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS));
    // TPMSG_NO_MSG_AVAILABLE,
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_OUT_OF_MEMORY),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));
    // TPMSG_SIZE_TOO_LARGE,
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_DIAGNOSTIC_MESSAGE_TOO_LARGE),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE));
    // TPMSG_NOT_RESPONSIBLE
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_UNKNOWN_NETWORK),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE));
}

TEST_F(DoIpTransportMessageProvidingListenerHelperTest, TestCreateGetResultFromNackCode)
{
    // NACK_DIAG_INVALID_SOURCE_ADDRESS
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_SRC_ADDRESS,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_SOURCE_ADDRESS),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_SOURCE_ADDRESS));
    // NACK_DIAG_INVALID_TARGET_ADDRESS
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_TARGET_ADDRESS),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_INVALID_TARGET_ADDRESS));
    // NACK_DIAG_DIAGNOSTIC_MESSAGE_TOO_LARGE
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_SIZE_TOO_LARGE,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_DIAGNOSTIC_MESSAGE_TOO_LARGE),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_DIAGNOSTIC_MESSAGE_TOO_LARGE));
    // NACK_DIAG_OUT_OF_MEMORY
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_OUT_OF_MEMORY),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_OUT_OF_MEMORY));
    // NACK_DIAG_TARGET_UNREACHABLE
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_INVALID_TGT_ADDRESS,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TARGET_UNREACHABLE),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TARGET_UNREACHABLE));
    // NACK_DIAG_TARGET_UNREACHABLE
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_UNKNOWN_NETWORK),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_UNKNOWN_NETWORK));
    // NACK_DIAG_TRANSPORT_PROTOCOL_ERROR
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::GetResult(
            ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TRANSPORT_PROTOCOL_ERROR),
        DoIpTransportMessageProvidingListenerHelper::createGetResult(
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TRANSPORT_PROTOCOL_ERROR));
}

TEST_F(DoIpTransportMessageProvidingListenerHelperTest, TestCreateReceiveResultFromReceiveResult)
{
    // RECEIVED_NO_ERROR
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::ReceiveResult(
            ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TRANSPORT_PROTOCOL_ERROR),
        DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
            ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));
    // RECEIVED_ERROR
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::ReceiveResult(
            ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TRANSPORT_PROTOCOL_ERROR),
        DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
            ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR));
}

TEST_F(DoIpTransportMessageProvidingListenerHelperTest, TestCreateReceiveResultFromNackCode)
{
    // NACK_DIAG_TRANSPORT_PROTOCOL_ERROR
    EXPECT_EQ(
        IDoIpTransportMessageProvidingListener::ReceiveResult(
            ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR,
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TRANSPORT_PROTOCOL_ERROR),
        DoIpTransportMessageProvidingListenerHelper::createReceiveResult(
            DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TRANSPORT_PROTOCOL_ERROR));
}

} // namespace test
} // namespace doip
