/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpConstants.h"
#include "doip/common/DoIpTransportMessageProvidingListenerAdapter.h"
#include "doip/common/DoIpTransportMessageProvidingListenerMock.h"

#include <transport/TransportMessage.h>
#include <transport/TransportMessageProcessedListenerMock.h>

namespace doip
{
namespace test
{
using namespace ::testing;

class DoIpTransportMessageProvidingListenerAdapterTest : public Test
{
public:
protected:
    DoIpTransportMessageProvidingListenerMock fProvidingListenerMock;
    ::transport::TransportMessage fTransportMessage;
    ::transport::TransportMessageProcessedListenerMock fTransportProcessedListenerMock;
};

TEST_F(DoIpTransportMessageProvidingListenerAdapterTest, TestForwarding)
{
    DoIpTransportMessageProvidingListenerAdapter cut(fProvidingListenerMock);
    // getTransportMessage
    ::transport::TransportMessage* transportMessage = nullptr;
    EXPECT_CALL(fProvidingListenerMock, getTransportMessage(0U, 123U, 234U, 100U, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(&fTransportMessage),
            Return(IDoIpTransportMessageProvidingListener::GetResult(
                ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE,
                DoIpConstants::DiagnosticMessageNackCodes::NACK_DIAG_TARGET_UNREACHABLE))));
    EXPECT_EQ(
        ::transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE,
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
        ::transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR,
        cut.messageReceived(0U, fTransportMessage, &fTransportProcessedListenerMock));
    // dump
    cut.dump();
}

} // namespace test
} // namespace doip
