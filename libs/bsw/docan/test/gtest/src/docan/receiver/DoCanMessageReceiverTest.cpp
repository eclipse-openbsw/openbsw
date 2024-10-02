// Copyright 2024 Accenture.

#include "docan/receiver/DoCanMessageReceiver.h"

#include "docan/common/DoCanConnection.h"
#include "docan/datalink/DoCanDataLinkAddressPair.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

#include <gmock/gmock.h>

namespace
{
using namespace docan;
using namespace transport;

using ReceiveProtocolHandler = DoCanMessageReceiveProtocolHandler<uint16_t>;

struct DataLinkLayer
{
    using AddressType     = uint32_t;
    using AddressPairType = DoCanDataLinkAddressPair<uint32_t>;
    using MessageSizeType = uint16_t;
    using FrameIndexType  = uint16_t;
    using FrameSizeType   = uint8_t;
    using JobHandleType   = uint16_t;
};

using CodecType = DoCanFrameCodec<DataLinkLayer>;

TEST(DoCanMessageReceiverTest, testConstructedReceiver)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {0x12, 0x34, 0x56, 0x78};
    DoCanMessageReceiver<DataLinkLayer> cut(connection, 123U, 8U, 12U, 4U, 2U, data, false);
    EXPECT_EQ(0, cut.getMessage());
    EXPECT_EQ(DoCanTransportAddressPair(0x34, 0x45), cut.getTransportAddressPair());
    EXPECT_EQ(0x123U, cut.getReceptionAddress());
    EXPECT_EQ(0x456U, cut.getTransmissionAddress());
    EXPECT_EQ(123U, cut.getMessageSize());
    EXPECT_EQ(4U, cut.getMaxBlockSize());
    EXPECT_EQ(2U, cut.getEncodedMinSeparationTime());
    EXPECT_FALSE(cut.isBlocked());
    cut.setBlocked(true);
    EXPECT_TRUE(cut.isBlocked());

    // set/update timers
    uint32_t nowUs = 100U;
    EXPECT_FALSE(cut.updateTimer(nowUs));
    cut.setTimer(300U);
    nowUs += 100U;
    EXPECT_FALSE(cut.updateTimer(nowUs));
    nowUs += 100U;
    EXPECT_TRUE(cut.updateTimer(nowUs));
    nowUs += 100U;
    EXPECT_FALSE(cut.updateTimer(nowUs));
    cut.setTimer(550U);
    nowUs += 50U;
    EXPECT_FALSE(cut.updateTimer(nowUs));
    nowUs += 51U;
    EXPECT_FALSE(cut.updateTimer(nowUs));
    nowUs += 49U;
    EXPECT_TRUE(cut.updateTimer(nowUs));

    // check release
    ::transport::TransportMessage message;
    cut.allocated(&message, 1U);
    EXPECT_EQ(&message, cut.getMessage());
    EXPECT_EQ(&message, cut.release());
    EXPECT_EQ(nullptr, cut.release());
}

TEST(DoCanMessageReceiverTest, testSingleFrameIsReceived)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {0x12, 0x34, 0x56, 0x78};
    DoCanMessageReceiver<DataLinkLayer> cut(connection, sizeof(data), 1U, 8U, 4U, 2U, data, false);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    uint8_t buffer[sizeof(data)];
    TransportMessage message(buffer, sizeof(buffer));
    EXPECT_EQ(ReceiveResult(true), cut.allocated(&message, 1U));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(sizeof(data), message.getValidBytes());
    EXPECT_EQ(0, ::memcmp(message.getPayload(), data, sizeof(data)));
    EXPECT_EQ(ReceiveResult(true), cut.processed(true));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
}

/**
 * \req: [BSW_DoCAN_112], [BSW_DoCAN_117], [BSW_DoCAN_118]
 */
TEST(DoCanMessageReceiverTest, testSingleFrameIsReceivedAfterDelayedAllocation)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {0x12, 0x34, 0x56, 0x78};
    DoCanMessageReceiver<DataLinkLayer> cut(connection, sizeof(data), 1U, 8U, 0U, 0U, data, false);
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    // timeout more than once?
    EXPECT_EQ(ReceiveResult(true), cut.allocated(0, 2U));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::ALLOCATE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.expired());
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    uint8_t buffer[sizeof(data)];
    TransportMessage message(buffer, sizeof(buffer));
    EXPECT_EQ(ReceiveResult(true), cut.allocated(&message, 2U));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(sizeof(data), message.getValidBytes());
    EXPECT_EQ(0, ::memcmp(message.getPayload(), data, sizeof(data)));
    EXPECT_EQ(ReceiveResult(true), cut.processed(true));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
}

TEST(DoCanMessageReceiverTest, testMultipleFramesAreReceived)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79};
    DoCanMessageReceiver<DataLinkLayer> cut(
        connection,
        sizeof(data),
        3U,
        7U,
        0x02U,
        0x3fU,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U),
        false);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    uint8_t buffer[sizeof(data)];
    TransportMessage message(buffer, sizeof(buffer));
    EXPECT_EQ(ReceiveResult(true), cut.allocated(&message, 1U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_FALSE(cut.isFlowControlWait());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_TRUE(cut.isConsecutiveFrameExpected());
    EXPECT_EQ(7U, cut.getExpectedConsecutiveFrameDataSize());
    EXPECT_EQ(
        ReceiveResult(true),
        cut.consecutiveFrameReceived(
            1U, 7U, ::estd::slice<uint8_t const>::from_pointer(data + 6U, 7U)));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_TRUE(cut.isConsecutiveFrameExpected());
    EXPECT_EQ(2U, cut.getExpectedConsecutiveFrameDataSize());
    EXPECT_EQ(
        ReceiveResult(true),
        cut.consecutiveFrameReceived(
            2U, 2U, ::estd::slice<uint8_t const>::from_pointer(data + 13U, 2U)));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(sizeof(data), message.getValidBytes());
    EXPECT_EQ(0, ::memcmp(message.getPayload(), data, sizeof(data)));
    EXPECT_EQ(ReceiveResult(true), cut.processed(true));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
}

/**
 * \req: [BSW_DoCAN_112]
 */
TEST(DoCanMessageReceiverTest, testMultipleFramesAreReceivedAfterDelayedAllocation)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79};
    DoCanMessageReceiver<DataLinkLayer> cut(
        connection,
        sizeof(data),
        3U,
        7U,
        0x03U,
        0x00,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U),
        false);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    // failed allocation
    EXPECT_EQ(ReceiveResult(true), cut.allocated(0, 2U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_TRUE(cut.isFlowControlWait());
    // failed send
    EXPECT_EQ(ReceiveResult(false), cut.frameSent(false));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    // successful send
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::ALLOCATE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.expired());
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    uint8_t buffer[sizeof(data)];
    TransportMessage message(buffer, sizeof(buffer));
    EXPECT_EQ(ReceiveResult(true), cut.allocated(&message, 2U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_FALSE(cut.isFlowControlWait());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_TRUE(cut.isConsecutiveFrameExpected());
    EXPECT_EQ(7U, cut.getExpectedConsecutiveFrameDataSize());
    EXPECT_EQ(
        ReceiveResult(true),
        cut.consecutiveFrameReceived(
            1U, 7U, ::estd::slice<uint8_t const>::from_pointer(data + 6U, 7U)));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(
        ReceiveResult(true),
        cut.consecutiveFrameReceived(
            2U, 2U, ::estd::slice<uint8_t const>::from_pointer(data + 13U, 2U)));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_TRUE(cut.isConsecutiveFrameExpected());
    EXPECT_EQ(sizeof(data), message.getValidBytes());
    EXPECT_EQ(0, ::memcmp(message.getPayload(), data, sizeof(data)));
    EXPECT_EQ(ReceiveResult(true), cut.processed(true));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
}

/**
 * \req: [BSW_DoCAN_115]
 */
TEST(DoCanMessageReceiverTest, testMultipleFramesAreReceivedWithBlockSize)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79};
    DoCanMessageReceiver<DataLinkLayer> cut(
        connection,
        sizeof(data),
        3U,
        7U,
        0x01U,
        0x03fU,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U),
        false);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    uint8_t buffer[sizeof(data)];
    TransportMessage message(buffer, sizeof(buffer));
    EXPECT_EQ(ReceiveResult(true), cut.allocated(&message, 2U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_FALSE(cut.isFlowControlWait());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_TRUE(cut.isConsecutiveFrameExpected());
    EXPECT_EQ(7U, cut.getExpectedConsecutiveFrameDataSize());
    EXPECT_EQ(
        ReceiveResult(true),
        cut.consecutiveFrameReceived(
            1U, 7U, ::estd::slice<uint8_t const>::from_pointer(data + 6U, 7U)));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_TRUE(cut.isConsecutiveFrameExpected());
    EXPECT_FALSE(cut.isFlowControlWait());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(2U, cut.getExpectedConsecutiveFrameDataSize());
    EXPECT_EQ(
        ReceiveResult(true),
        cut.consecutiveFrameReceived(
            2U, 2U, ::estd::slice<uint8_t const>::from_pointer(data + 13U, 2U)));
    EXPECT_EQ(ReceiveState::PROCESSING, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_EQ(sizeof(data), message.getValidBytes());
    EXPECT_EQ(0, ::memcmp(message.getPayload(), data, sizeof(data)));
    EXPECT_EQ(ReceiveResult(true), cut.processed(true));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
}

/**
 * \req: [BSW_DoCAN_149]
 */
TEST(DoCanMessageReceiverTest, testConsecutiveFrameWithBadSequenceNumberIsReceived)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79};
    DoCanMessageReceiver<DataLinkLayer> cut(
        connection,
        sizeof(data),
        3U,
        7U,
        1U,
        0x3fU,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U),
        false);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    uint8_t buffer[sizeof(data)];
    TransportMessage message(buffer, sizeof(buffer));
    EXPECT_EQ(ReceiveResult(true), cut.allocated(&message, 2U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_FALSE(cut.isFlowControlWait());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::RX, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
    EXPECT_TRUE(cut.isConsecutiveFrameExpected());
    EXPECT_EQ(7U, cut.getExpectedConsecutiveFrameDataSize());
    EXPECT_EQ(
        ReceiveResult(true).setMessage(ReceiveMessage::BAD_SEQUENCE_NUMBER, 2U),
        cut.consecutiveFrameReceived(
            2U, 7U, ::estd::slice<uint8_t const>::from_pointer(data + 6U, 7U)));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_FALSE(cut.isAllocating());
}

TEST(DoCanMessageReceiverTest, testConsecutiveFrameIsReceivedDuringAllocation)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79};
    DoCanMessageReceiver<DataLinkLayer> cut(
        connection,
        sizeof(data),
        3U,
        7U,
        0U,
        0U,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U),
        false);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_FALSE(cut.isConsecutiveFrameExpected());
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
}

TEST(DoCanMessageReceiverTest, testTimeoutIfAllocationFailsRepeatedly)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79};
    DoCanMessageReceiver<DataLinkLayer> cut(
        connection,
        sizeof(data),
        3U,
        7U,
        0U,
        0U,
        ::estd::slice<uint8_t const>::from_pointer(data, 6U),
        false);
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.allocated(0, 2U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::ALLOCATE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.expired());
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.allocated(0, 2U));
    EXPECT_EQ(ReceiveState::SEND, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.frameSent(true));
    EXPECT_EQ(ReceiveState::WAIT, cut.getState());
    EXPECT_EQ(ReceiveTimeout::ALLOCATE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(ReceiveResult(true), cut.expired());
    EXPECT_EQ(ReceiveState::ALLOCATE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
    EXPECT_TRUE(cut.isAllocating());
    EXPECT_EQ(
        ReceiveResult(true).setMessage(ReceiveMessage::ALLOCATION_RETRY_COUNT_EXCEEDED),
        cut.allocated(0, 2U));
    EXPECT_EQ(ReceiveState::DONE, cut.getState());
    EXPECT_EQ(ReceiveTimeout::NONE, cut.getTimeout());
}

TEST(DoCanMessageReceiverTest, testTimers)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    DoCanConnection<DataLinkLayer> connection(
        codec, DataLinkLayer::AddressPairType(0x123, 0x456), DoCanTransportAddressPair(0x34, 0x45));
    uint8_t const data[] = {0x12, 0x34, 0x56, 0x78};
    DoCanMessageReceiver<DataLinkLayer> cut1(connection, 123U, 8U, 12U, 4U, 2U, data, false);
    DoCanMessageReceiver<DataLinkLayer> cut2(connection, 123U, 8U, 12U, 4U, 2U, data, false);

    uint32_t nowUs = 100U;
    EXPECT_FALSE(cut1.updateTimer(nowUs));
    EXPECT_FALSE(cut2.updateTimer(nowUs));
    // Both should be unset, neither is less than the other
    EXPECT_FALSE(cut1 < cut2);
    EXPECT_FALSE(cut2 < cut1);
    cut1.setTimer(300U);
    cut2.setTimer(400U);
    // Both set, 1 smaller than 2
    EXPECT_TRUE(cut1 < cut2);
    EXPECT_FALSE(cut2 < cut1);
    nowUs += 100U;
    EXPECT_FALSE(cut1.updateTimer(nowUs));
    EXPECT_FALSE(cut2.updateTimer(nowUs));
    nowUs += 100U;
    EXPECT_TRUE(cut1.updateTimer(nowUs));
    EXPECT_FALSE(cut2.updateTimer(nowUs));
    // Only 2 set, 2 smaller than 1
    EXPECT_TRUE(cut2 < cut1);
    EXPECT_FALSE(cut1 < cut2);
    nowUs += 100U;
    EXPECT_FALSE(cut1.updateTimer(nowUs));
    EXPECT_TRUE(cut2.updateTimer(nowUs));
    // Both unset again, neither less than the other
    EXPECT_FALSE(cut1 < cut2);
    EXPECT_FALSE(cut2 < cut1);
}

} // anonymous namespace
