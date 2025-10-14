// Copyright 2024 Accenture.

#include "docan/receiver/DoCanMessageReceiveProtocolHandler.h"

#include <gmock/gmock.h>

namespace
{
using namespace docan;
using ReceiveProtocolHandler = DoCanMessageReceiveProtocolHandler<uint16_t>;

TEST(DoCanMessageReceiveProtocolHandlerTest, testResultClass)
{
    {
        // Default constructor
        ReceiveResult cut(true);
        EXPECT_TRUE(cut.hasTransition());
        EXPECT_EQ(ReceiveMessage::NONE, cut.getMessage());
        EXPECT_EQ(0U, cut.getParam());
        // set message
        cut.setMessage(ReceiveMessage::ILLEGAL_STATE);
        EXPECT_TRUE(cut.hasTransition());
        EXPECT_EQ(ReceiveMessage::ILLEGAL_STATE, cut.getMessage());
        EXPECT_EQ(0U, cut.getParam());
        // set message with param
        cut.setMessage(ReceiveMessage::BAD_SEQUENCE_NUMBER, 2U);
        EXPECT_TRUE(cut.hasTransition());
        EXPECT_EQ(ReceiveMessage::BAD_SEQUENCE_NUMBER, cut.getMessage());
        EXPECT_EQ(2U, cut.getParam());
    }
    {
        // operator== is used explicitly, this is the method to test (EXPECT_NE doesn't work here)
        EXPECT_TRUE(
            ReceiveResult(true).setMessage(ReceiveMessage::ILLEGAL_STATE, 0x7fU)
            == ReceiveResult(true).setMessage(ReceiveMessage::ILLEGAL_STATE, 0x7fU));
        EXPECT_FALSE(
            ReceiveResult(false).setMessage(ReceiveMessage::ILLEGAL_STATE, 0x7fU)
            == ReceiveResult(true).setMessage(ReceiveMessage::ILLEGAL_STATE, 0x7fU));
        EXPECT_FALSE(
            ReceiveResult(true).setMessage(ReceiveMessage::NONE, 0x7fU)
            == ReceiveResult(true).setMessage(ReceiveMessage::ILLEGAL_STATE, 0x7fU));
        EXPECT_FALSE(
            ReceiveResult(true).setMessage(ReceiveMessage::ILLEGAL_STATE, 0x7eU)
            == ReceiveResult(true).setMessage(ReceiveMessage::ILLEGAL_STATE, 0x7fU));
    }
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateIdleAfterConstruction)
{
    ReceiveProtocolHandler cut(0U);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(1U, cut.getFrameIndex());
    EXPECT_EQ(0U, cut.getFrameCount());
    EXPECT_TRUE(cut.isAllocating());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateAllocateAfterInit)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(1U, cut.getFrameIndex());
    EXPECT_EQ(1U, cut.getFrameCount());
    EXPECT_TRUE(cut.isAllocating());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateProcessingAfterSuccessfulAllocation)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(1U, cut.getFrameIndex());
    EXPECT_EQ(1U, cut.getFrameCount());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_FALSE(cut.isFlowControlWait());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateDoneAfterSuccessfulProcessing)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveResult(true), cut.processed(true));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(1U, cut.getFrameIndex());
    EXPECT_EQ(1U, cut.getFrameCount());
    EXPECT_FALSE(cut.isAllocating());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateDoneAfterFailedProcessing)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(
        ReceiveResult(true).setMessage(ReceiveMessage::PROCESSING_FAILED), cut.processed(false));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(1U, cut.getFrameIndex());
    EXPECT_EQ(1U, cut.getFrameCount());
    EXPECT_FALSE(cut.isAllocating());
}

/**
 * \req: [BSW_DoCAN_106], [BSW_DoCAN_107]
 */
TEST(DoCanMessageReceiveProtocolHandlerTest, testStateReceiveForEachConsecutiveFrame)
{
    ReceiveProtocolHandler cut(3U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_EQ(1U, cut.getFrameIndex());
    EXPECT_EQ(3U, cut.getFrameCount());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.consecutiveFrameReceived(1U, 0U));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_EQ(2U, cut.getFrameIndex());
    EXPECT_EQ(3U, cut.getFrameCount());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.consecutiveFrameReceived(2U, 0U));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(3U, cut.getFrameIndex());
    EXPECT_EQ(3U, cut.getFrameCount());
}

/**
 * \req: [BSW_DoCAN_106], [BSW_DoCAN_107]
 */
TEST(DoCanMessageReceiveProtocolHandlerTest, testStateSendAfterEndOfBlock)
{
    ReceiveProtocolHandler cut(4U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_EQ(1U, cut.getFrameIndex());
    EXPECT_EQ(4U, cut.getFrameCount());
    EXPECT_EQ(ReceiveResult(true), cut.consecutiveFrameReceived(1U, 2U));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_EQ(2U, cut.getFrameIndex());
    EXPECT_EQ(4U, cut.getFrameCount());
    EXPECT_EQ(ReceiveResult(true), cut.consecutiveFrameReceived(2U, 2U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(3U, cut.getFrameIndex());
    EXPECT_EQ(4U, cut.getFrameCount());
    EXPECT_EQ(ReceiveResult(true), cut.consecutiveFrameReceived(3U, 0U));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(4U, cut.getFrameIndex());
    EXPECT_EQ(4U, cut.getFrameCount());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateDoneAfterReceptionOfBadSequenceNumber)
{
    ReceiveProtocolHandler cut(3U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.consecutiveFrameReceived(1U, 0U));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(
        ReceiveResult(true).setMessage(ReceiveMessage::BAD_SEQUENCE_NUMBER, 1U),
        cut.consecutiveFrameReceived(1U, 0U));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateDoneAfterSegmentedAllocationTimeout)
{
    ReceiveProtocolHandler cut(3U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(false, 2U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_TRUE(cut.isFlowControlWait());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::ALLOCATE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.expired());
    EXPECT_EQ(ReceiveResult(true), cut.allocated(false, 2U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_TRUE(cut.isFlowControlWait());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::ALLOCATE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.expired());
    EXPECT_EQ(
        ReceiveResult(true).setMessage(ReceiveMessage::ALLOCATION_RETRY_COUNT_EXCEEDED),
        cut.allocated(false, 2U));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateDoneAfterSingleFrameAllocationTimeout)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(false, 2U));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::ALLOCATE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_FALSE(cut.isFlowControlWait());
    EXPECT_EQ(
        ReceiveResult(true).setMessage(ReceiveMessage::ALLOCATION_RETRY_COUNT_EXCEEDED),
        cut.allocated(false, 2U));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateDoneAfterReceptionTimeout)
{
    ReceiveProtocolHandler cut(4U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_EQ(ReceiveResult(true).setMessage(ReceiveMessage::RX_TIMEOUT_EXPIRED), cut.expired());
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateDoneAfterCancel)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(
        ReceiveResult(true).setMessage(ReceiveMessage::ILLEGAL_STATE),
        cut.cancel(ReceiveMessage::ILLEGAL_STATE));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testNoStateTransitionIfShutdownDuringProcessing)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(ReceiveResult(false), cut.shutdown());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testStateTransitionIfShutdownDuringAllocation)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(ReceiveResult(true), cut.shutdown());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testIllegalStateMessageIfFrameSentInWrongState)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(
        ReceiveResult(true).setMessage(
            ReceiveMessage::ILLEGAL_STATE, static_cast<uint8_t>(ReceiveState::PROCESSING)),
        cut.frameSent(true));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
}

/**
 * \req: [BSW_DoCAN_159]
 */
TEST(DoCanMessageReceiveProtocolHandlerTest, testConsecutiveFrameReceivedInWrongStateIsIgnored)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveResult(false), cut.consecutiveFrameReceived(1U, 0U));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
}

TEST(DoCanMessageReceiveProtocolHandlerTest, testConsecutiveFrameReceivedBeforeAllocatedIsIgnored)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(false, 1U));
    EXPECT_EQ(ReceiveResult(false), cut.consecutiveFrameReceived(1U, 0U));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::ALLOCATE, cut.getTimeout());
}

TEST(
    DoCanMessageReceiveProtocolHandlerTest,
    testIllegalStateMessageIfAllocatingDuringNonAllocatingState)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(
        ReceiveResult(true).setMessage(
            ReceiveMessage::ILLEGAL_STATE, static_cast<uint8_t>(ReceiveState::PROCESSING)),
        cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
}

TEST(
    DoCanMessageReceiveProtocolHandlerTest,
    testStateNoStateTransitionIfTimeoutExpiredDuringNonTimeoutState)
{
    ReceiveProtocolHandler cut(1U);
    EXPECT_EQ(ReceiveResult(true), cut.allocated(true, 1U));
    EXPECT_EQ(ReceiveResult(false), cut.expired());
}

} // anonymous namespace
