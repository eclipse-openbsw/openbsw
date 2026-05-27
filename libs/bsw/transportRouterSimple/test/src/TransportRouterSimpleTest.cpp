/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "transport/routing/TransportRouterSimple.h"
#include "transport/AbstractTransportLayerMock.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportMessage.h"

#include <async/LockMock.h>
#include <busid/BusId.h>

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::transport;

class TransportRouterSimpleTest : public ::testing::Test
{
protected:
    TransportRouterSimpleTest()
    : _ethTransportLayer(::busid::ETH_0), _selfDiagTransportLayer(::busid::SELFDIAG)
    {}

    void SetUp() override
    {
        _router.init();
        _router.addTransportLayer(_ethTransportLayer);
        _router.addTransportLayer(_selfDiagTransportLayer);
    }

    void TearDown() override
    {
        _router.removeTransportLayer(_ethTransportLayer);
        _router.removeTransportLayer(_selfDiagTransportLayer);
        _router.shutdown();
    }

    void initMessage(TransportMessage& msg, uint8_t* buffer, uint16_t bufferSize)
    {
        msg.init(buffer, bufferSize);
    }

    TransportRouterSimple _router;
    StrictMock<AbstractTransportLayerMock> _ethTransportLayer;
    StrictMock<AbstractTransportLayerMock> _selfDiagTransportLayer;
    async::LockMock _lockMock;

    static uint16_t const BUFFER_SIZE = 64U;
};

/**
 * Test a full request/response round-trip across the ETH <-> SELFDIAG boundary.
 *
 * Inbound (ETH_0 -> SELFDIAG):
 *   ETH_0 is a 2-byte bus, SELFDIAG is a 1-byte bus.
 *   The router converts 2-byte addresses to 1-byte when forwarding to SELFDIAG.
 *
 * Outbound (SELFDIAG -> ETH_0):
 *   The router converts 1-byte addresses to 2-byte on receive from SELFDIAG,
 *   then forwards to ETH_0 without further conversion.
 */
TEST_F(TransportRouterSimpleTest, requestFromETH_responseFromSelfDiag_roundTrip)
{
    // --- Inbound: ETH_0 -> SELFDIAG ---
    uint8_t requestBuffer[BUFFER_SIZE];
    TransportMessage requestMsg;
    initMessage(requestMsg, requestBuffer, BUFFER_SIZE);

    // 2-byte tester address (Ethernet tester #1: 0x0ECD -> 0x00F0)
    uint16_t const tester2Byte = 0x0ECDU;
    uint16_t const tester1Byte = 0x00F0U;
    uint16_t const ecuAddress  = 0x0006U;

    requestMsg.setSourceAddress(tester2Byte);
    requestMsg.setTargetAddress(ecuAddress);

    // When forwarding to SELFDIAG (1-byte bus), addresses are converted:
    //   source: 0x0ECD -> 0x00F0
    //   target: 0x0006 -> 0x0006 (no mapping, not a 0x0E__ address)
    EXPECT_CALL(_selfDiagTransportLayer, send(_, _))
        .WillOnce(Invoke(
            [&](TransportMessage& msg, ITransportMessageProcessedListener*)
            {
                EXPECT_EQ(tester1Byte, msg.getSourceId());
                EXPECT_EQ(ecuAddress, msg.getTargetId());
                return AbstractTransportLayer::ErrorCode::TP_OK;
            }));

    auto result = _router.messageReceived(::busid::ETH_0, requestMsg, nullptr);
    EXPECT_EQ(ITransportMessageProvidingListener::ReceiveResult::RECEIVED_NO_ERROR, result);

    // --- Outbound: SELFDIAG -> ETH_0 (reply) ---
    uint8_t responseBuffer[BUFFER_SIZE];
    TransportMessage responseMsg;
    initMessage(responseMsg, responseBuffer, BUFFER_SIZE);

    // The ECU responds with 1-byte addresses
    responseMsg.setSourceAddress(ecuAddress);
    responseMsg.setTargetAddress(tester1Byte);

    // On receive from SELFDIAG (1-byte bus), addresses are converted to 2-byte:
    //   source: 0x0006 -> 0x0006 (no mapping)
    //   target: 0x00F0 -> 0x0ECD
    // When forwarding to ETH_0 (2-byte bus), no conversion:
    EXPECT_CALL(_ethTransportLayer, send(_, _))
        .WillOnce(Invoke(
            [&](TransportMessage& msg, ITransportMessageProcessedListener*)
            {
                EXPECT_EQ(ecuAddress, msg.getSourceId());
                EXPECT_EQ(tester2Byte, msg.getTargetId());
                return AbstractTransportLayer::ErrorCode::TP_OK;
            }));

    result = _router.messageReceived(::busid::SELFDIAG, responseMsg, nullptr);
    EXPECT_EQ(ITransportMessageProvidingListener::ReceiveResult::RECEIVED_NO_ERROR, result);
}

/**
 * Test that getTransportMessage correctly translates the functional address
 * check when the request arrives from a 1-byte bus (CAN_0).
 *
 * The functional address 0x00DF does not match the 0x00F_ prefix guard,
 * so convert1ByteAddressTo2Byte passes it through unchanged, and
 * isFunctionalAddress(0x00DF) correctly matches FUNCTIONAL_ALL_ISO14229.
 */
TEST_F(TransportRouterSimpleTest, getTransportMessage_fromCAN_functionalAddress)
{
    TransportMessage* pMsg = nullptr;

    auto const errorCode = _router.getTransportMessage(
        ::busid::CAN_0,
        0x00F3U,                                                     // source (1-byte tester)
        TransportConfiguration::FUNCTIONAL_ALL_ISO14229,             // target (functional)
        TransportConfiguration::MAX_FUNCTIONAL_MESSAGE_PAYLOAD_SIZE, // size
        {},
        pMsg);

    EXPECT_EQ(ITransportMessageProvidingListener::ErrorCode::TPMSG_OK, errorCode);
    EXPECT_NE(nullptr, pMsg);

    if (pMsg != nullptr)
    {
        _router.releaseTransportMessage(*pMsg);
    }
}

/**
 * Test that getTransportMessage returns TPMSG_SIZE_TOO_LARGE when the
 * requested message size exceeds the router's buffer capacity.
 */
TEST_F(TransportRouterSimpleTest, getTransportMessage_sizeTooLarge)
{
    TransportMessage* pMsg = nullptr;

    auto const errorCode = _router.getTransportMessage(
        ::busid::ETH_0,
        0x0ECDU,
        0x0006U,
        TransportRouterSimple::BUFFER_SIZE + 1, // too large
        {},
        pMsg);

    EXPECT_EQ(ITransportMessageProvidingListener::ErrorCode::TPMSG_SIZE_TOO_LARGE, errorCode);
    EXPECT_EQ(nullptr, pMsg);
}

/**
 * Test that messageReceived returns RECEIVED_ERROR when the message
 * comes from SELFDIAG but no prior external request set _busIdToReply
 * to anything other than SELFDIAG.
 */
TEST_F(TransportRouterSimpleTest, messageReceived_fromSelfDiag_noReplyTarget)
{
    uint8_t buffer[BUFFER_SIZE];
    TransportMessage msg;
    initMessage(msg, buffer, BUFFER_SIZE);
    msg.setSourceAddress(0x0006U);
    msg.setTargetAddress(0x00F0U);

    auto const result = _router.messageReceived(::busid::SELFDIAG, msg, nullptr);
    EXPECT_EQ(ITransportMessageProvidingListener::ReceiveResult::RECEIVED_ERROR, result);
}

} // namespace
