// Copyright 2024 Accenture.

#include "docan/transmitter/DoCanMessageTransmitProtocolHandler.h"

#include "docan/common/DoCanConstants.h"

#include <gmock/gmock.h>

namespace
{
using namespace docan;
using TransmitProtocolHandler = DoCanMessageTransmitProtocolHandler<uint16_t>;
// NOLINTBEGIN(cert-err58-cpp): Lots of references to these names in this file, as such suppress was
// prefered here since it's just a test file.
static TransmitActionSetType storeSeparationTime
    = TransmitActionSetType().set(TransmitAction::STORE_SEPARATION_TIME);
static TransmitActionSetType cancelSend = TransmitActionSetType().set(TransmitAction::CANCEL_SEND);

// NOLINTEND(cert-err58-cpp)

TEST(DoCanMessageTransmitProtocolHandlerTest, testResultClass)
{
    TransmitActionSetType noAction;
    TransmitActionSetType storeAction = storeSeparationTime;
    {
        TransmitResult cut(false);
        EXPECT_FALSE(cut.hasTransition());
        EXPECT_EQ(noAction, cut.getActionSet());
        EXPECT_EQ(TransmitMessage::NONE, cut.getMessage());
        EXPECT_EQ(0U, cut.getParam());
        // Set action set
        cut.setActionSet(storeAction);
        EXPECT_EQ(storeAction, cut.getActionSet());
        // set message with parameter
        cut.setMessage(TransmitMessage::ILLEGAL_STATE, 2U);
        EXPECT_EQ(TransmitMessage::ILLEGAL_STATE, cut.getMessage());
        EXPECT_EQ(2U, cut.getParam());
        // set message
        cut.setMessage(TransmitMessage::FLOW_CONTROL_TIMEOUT_EXPIRED);
        EXPECT_EQ(TransmitMessage::FLOW_CONTROL_TIMEOUT_EXPIRED, cut.getMessage());
        EXPECT_EQ(0U, cut.getParam());
    }
    {
        // operator== is used explicitly, this is the method to test (EXPECT_NE doesn't work here)
        EXPECT_TRUE(
            TransmitResult(true)
                .setActionSet(storeAction)
                .setMessage(TransmitMessage::ILLEGAL_STATE, uint8_t(0x7fU))
            == TransmitResult(true)
                   .setActionSet(storeAction)
                   .setMessage(TransmitMessage::ILLEGAL_STATE, uint8_t(0x7fU)));
        EXPECT_FALSE(
            TransmitResult(false)
                .setActionSet(storeAction)
                .setMessage(TransmitMessage::ILLEGAL_STATE, uint8_t(0x7fU))
            == TransmitResult(true)
                   .setActionSet(storeAction)
                   .setMessage(TransmitMessage::ILLEGAL_STATE, uint8_t(0x7fU)));
        EXPECT_FALSE(
            TransmitResult(true).setActionSet(noAction).setMessage(
                TransmitMessage::ILLEGAL_STATE, uint8_t(0x7fU))
            == TransmitResult(true)
                   .setActionSet(storeAction)
                   .setMessage(TransmitMessage::ILLEGAL_STATE, uint8_t(0x7fU)));
        EXPECT_FALSE(
            TransmitResult(true)
                .setActionSet(storeAction)
                .setMessage(TransmitMessage::NONE, uint8_t(0x7fU))
            == TransmitResult(true)
                   .setActionSet(storeAction)
                   .setMessage(TransmitMessage::ILLEGAL_STATE, uint8_t(0x7fU)));
        EXPECT_FALSE(
            TransmitResult(true)
                .setActionSet(storeAction)
                .setMessage(TransmitMessage::ILLEGAL_STATE, uint8_t(0x7eU))
            == TransmitResult(true)
                   .setActionSet(storeAction)
                   .setMessage(TransmitMessage::ILLEGAL_STATE, uint8_t(0x7fU)));
    }
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateIdleAfterConstruction)
{
    TransmitProtocolHandler cut(22U);
    EXPECT_EQ(TransmitState::INITIALIZED, cut.getState());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(0U, cut.getFrameIndex());
    EXPECT_EQ(1U, cut.getBlockEnd());
    EXPECT_EQ(22U, cut.getFrameCount());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateSendAfterStart)
{
    TransmitProtocolHandler cut(1U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateWaitAfterFrameSending)
{
    TransmitProtocolHandler cut(1U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateSuccessAfterFrameSent)
{
    TransmitProtocolHandler cut(1U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::SUCCESS, cut.getState());
}

TEST(CanTransportIsoMessageReceiveProtocolHandlerTest, testStateDoneAfterCancel)
{
    TransmitProtocolHandler cut(1U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    auto const result = cut.cancel(TransmitMessage::TX_CALLBACK_TIMEOUT_EXPIRED);
    // Regression test, previously STORE_SEPARATION_TIME and CANCEL_SEND was both set to 0U
    EXPECT_FALSE(
        TransmitResult(true)
            .setMessage(TransmitMessage::TX_CALLBACK_TIMEOUT_EXPIRED)
            .setActionSet(storeSeparationTime)
        == result);
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::FAIL, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateUnchangedAfterFrameSentInIllegalState)
{
    TransmitProtocolHandler cut(1U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(
        TransmitResult(true).setMessage(TransmitMessage::ILLEGAL_STATE, TransmitState::SEND),
        cut.framesSent(1U));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateUnchangedAfterFrameSentInIllegalWaitState)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitResult(true), cut.handleFlowControl(FlowStatus::WAIT, 1, true, 2));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
    EXPECT_EQ(
        TransmitResult(true).setMessage(TransmitMessage::ILLEGAL_STATE, TransmitState::WAIT),
        cut.framesSent(1U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateFailedAfterFrameSendingInIllegalState)
{
    TransmitProtocolHandler cut(1U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(
        TransmitResult(true)
            .setMessage(TransmitMessage::ILLEGAL_STATE, TransmitState::WAIT)
            .setActionSet(cancelSend),
        cut.frameSending());
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::FAIL, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateFailedAfterTxTimeout)
{
    TransmitProtocolHandler cut(1U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(
        TransmitResult(true)
            .setMessage(TransmitMessage::TX_CALLBACK_TIMEOUT_EXPIRED)
            .setActionSet(cancelSend),
        cut.expired());
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::FAIL, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateWaitForFlowControlAfterStartOfSegmentedSend)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateSendAfterFlowControlContinue)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(
        TransmitResult(true).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 1, true, 2));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateWaitAfterFlowControlWait)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitResult(true), cut.handleFlowControl(FlowStatus::WAIT, 1, true, 2));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
}

/**
 * \req: [BSW_DoCAN_124]
 */
TEST(DoCanMessageTransmitProtocolHandlerTest, testStateFailedAfterFlowControlOverflow)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(
        TransmitResult(true).setMessage(TransmitMessage::FLOW_CONTROL_OVERFLOW),
        cut.handleFlowControl(FlowStatus::OVFLW, 1, true, 2));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::FAIL, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateFailedAfterInvalidFlowControl)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(
        TransmitResult(true).setMessage(TransmitMessage::FLOW_CONTROL_INVALID),
        cut.handleFlowControl(static_cast<FlowStatus>(4U), 1, true, 2));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::FAIL, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateFailedAfterFlowControlWaitCountExceeded)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitResult(true), cut.handleFlowControl(FlowStatus::WAIT, 0, false, 2));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.handleFlowControl(FlowStatus::WAIT, 0, false, 2));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
    EXPECT_EQ(
        TransmitResult(true).setMessage(TransmitMessage::FLOW_CONTROL_WAIT_COUNT_EXCEEDED),
        cut.handleFlowControl(FlowStatus::WAIT, 0, false, 2));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::FAIL, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testFlowControlIsIgnored)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(false), cut.handleFlowControl(FlowStatus::WAIT, 1, true, 2));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
}

/**
 * \req: [BSW_DoCAN_160], [BSW_DoCAN_162]
 */
TEST(DoCanMessageTransmitProtocolHandlerTest, testFlowControlIgnoredInIllegalWaitState)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(
        TransmitResult(false).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 0, true, 2));
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::SEPARATION_TIME, cut.getTimeout());
    EXPECT_EQ(TransmitResult(false), cut.handleFlowControl(FlowStatus::WAIT, 1, true, 2));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::SEPARATION_TIME, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testFlowControlIgnoredIfSecondFlowControlReceived)
{
    TransmitProtocolHandler cut(2U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(
        TransmitResult(true).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 0, true, 2));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(false), cut.handleFlowControl(FlowStatus::CTS, 0, true, 2));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateSendIfFlowControlCtsBeforeTransmitCallback)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(
        TransmitResult(false).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 1, true, 2));
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateWaitIfFlowControlWaitBeforeTransmitCallback)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitResult(false), cut.handleFlowControl(FlowStatus::WAIT, 1, true, 2));
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateSendForConsecutiveFrames)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(
        TransmitResult(false).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 0, false, 2));
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::SUCCESS, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateSendForConsecutiveFramesWithSeparationTime)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(
        TransmitResult(false).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 0, true, 2));
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::SEPARATION_TIME, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.expired());
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::SUCCESS, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateSendForConsecutiveFramesWithBlockSize)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(
        TransmitResult(false).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 1, false, 2));
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
    EXPECT_EQ(
        TransmitResult(true).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 1, false, 2));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::SUCCESS, cut.getState());
}

/**
 * \req: [BSW_DoCAN_79]
 */
TEST(
    DoCanMessageTransmitProtocolHandlerTest, testStateSendForMultipleConsecutiveFramesWithBlockSize)
{
    TransmitProtocolHandler cut(16U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(
        TransmitResult(false).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 4, false, 2));
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(3U));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
    EXPECT_EQ(
        TransmitResult(true).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 7U, false, 2));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(7U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
    EXPECT_EQ(
        TransmitResult(true).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 0, false, 2));
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(7U));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::SUCCESS, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testStateFailedAfterFlowControlTimeout)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(
        TransmitResult(false).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 1, false, 2));
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
    EXPECT_EQ(
        TransmitResult(true).setMessage(TransmitMessage::FLOW_CONTROL_TIMEOUT_EXPIRED),
        cut.expired());
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::FAIL, cut.getState());
}

TEST(CanTransportIsoMessageReceiveProtocolHandlerTest, testStateDoneAfterCancelDuringInitialization)
{
    TransmitProtocolHandler cut(1U);
    EXPECT_EQ(
        TransmitResult(true).setMessage(TransmitMessage::TX_CALLBACK_TIMEOUT_EXPIRED),
        cut.cancel(TransmitMessage::TX_CALLBACK_TIMEOUT_EXPIRED));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::FAIL, cut.getState());
}

TEST(DoCanMessageTransmitProtocolHandlerTest, testNoStateTransitionIfUnexpectedTimeout)
{
    TransmitProtocolHandler cut(3U);
    EXPECT_EQ(TransmitResult(false), cut.expired());
    EXPECT_EQ(TransmitState::INITIALIZED, cut.getState());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
}

} // anonymous namespace
