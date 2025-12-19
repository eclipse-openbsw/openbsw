// Copyright 2024 Accenture.

#include "docan/transmitter/DoCanMessageTransmitter.h"

#include "docan/common/DoCanConstants.h"
#include "docan/common/DoCanJobHandle.h"
#include "docan/datalink/DoCanDataLinkAddressPair.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

#include <transport/TransportMessageProcessedListenerMock.h>

namespace
{
using namespace docan;
using namespace transport;

struct DataLinkLayer
{
    static uint32_t const INVALID_ADDRESS = 0xffffffffU;

    using AddressType     = uint32_t;
    using AddressPairType = DoCanDataLinkAddressPair<uint32_t>;
    using MessageSizeType = uint16_t;
    using FrameIndexType  = uint16_t;
    using FrameSizeType   = uint8_t;
    using JobHandleType   = DoCanJobHandle<uint16_t, uint16_t>;
};

using TransmitProtocolHandler = DoCanMessageTransmitProtocolHandler<DataLinkLayer::FrameIndexType>;
using JobHandle               = DataLinkLayer::JobHandleType;
using CodecType               = DoCanFrameCodec<DataLinkLayer>;

static TransmitActionSetType storeSeparationTime
    = TransmitActionSetType().set(TransmitAction::STORE_SEPARATION_TIME);

TEST(DoCanMessageTransmitterTest, testConstructedTransmitter)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    TransportMessageProcessedListenerMock listenerMock;
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9a};
    TransportMessage message(data, sizeof(data));
    message.setPayloadLength(sizeof(data));
    JobHandle jobHandle(6, 7);
    DoCanMessageTransmitter<DataLinkLayer> cut(
        jobHandle,
        codec,
        DataLinkLayer::AddressPairType(0x123, 0x456),
        message,
        &listenerMock,
        3U,
        15U);
    EXPECT_EQ(jobHandle, cut.getJobHandle());
    EXPECT_EQ(&message, &cut.getMessage());
    EXPECT_EQ(&listenerMock, cut.getNotificationListener());
    EXPECT_EQ(0x123U, cut.getReceptionAddress());
    EXPECT_EQ(0x456U, cut.getTransmissionAddress());
    EXPECT_EQ(0, cut.getMinSeparationTimeUs());
    EXPECT_EQ(0U, cut.getFrameIndex());
    EXPECT_EQ(1U, cut.getBlockEnd());
    EXPECT_EQ(3U, cut.getFrameCount());
    EXPECT_EQ(15U, cut.getConsecutiveFrameDataSize());
    EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(data), cut.getSendData()));

    EXPECT_FALSE(cut.updateTimer(100U));
    cut.setTimer(200U);
    EXPECT_FALSE(cut.updateTimer(100U));
    EXPECT_TRUE(cut.updateTimer(200U));
    EXPECT_FALSE(cut.updateTimer(300U));
    EXPECT_FALSE(cut.updateTimer(400U));
    cut.setTimer(400U);
    EXPECT_TRUE(cut.updateTimer(500U));
    EXPECT_FALSE(cut.updateTimer(700U));
}

TEST(DoCanMessageTransmitterTest, testSimpleCancel)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9a};
    TransportMessage message(data, sizeof(data));
    message.setPayloadLength(sizeof(data));
    JobHandle jobHandle(1, 2);
    DoCanMessageTransmitter<DataLinkLayer> cut(
        jobHandle, codec, DataLinkLayer::AddressPairType(0x123, 0x456), message, nullptr, 1U, 7U);
    EXPECT_EQ(TransmitState::INITIALIZED, cut.getState());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.cancel());
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::FAIL, cut.getState());
    cut.release();
    EXPECT_EQ(uint32_t(DataLinkLayer::INVALID_ADDRESS), cut.getReceptionAddress());
    EXPECT_EQ(uint32_t(DataLinkLayer::INVALID_ADDRESS), cut.getTransmissionAddress());
}

/**
 * \req: [BSW_DoCAN_72]
 */
TEST(DoCanMessageTransmitterTest, testSingleFrameIsEmitted)
{
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9a};
    TransportMessage message(data, sizeof(data));
    message.setPayloadLength(sizeof(data));
    JobHandle jobHandle(1, 2);
    DoCanMessageTransmitter<DataLinkLayer> cut(
        jobHandle, codec, DataLinkLayer::AddressPairType(0x123, 0x456), message, nullptr, 1U, 7U);
    EXPECT_EQ(TransmitState::INITIALIZED, cut.getState());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(data), cut.getSendData()));
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U, sizeof(data)));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::SUCCESS, cut.getState());
    cut.release();
    EXPECT_EQ(uint32_t(DataLinkLayer::INVALID_ADDRESS), cut.getReceptionAddress());
    EXPECT_EQ(uint32_t(DataLinkLayer::INVALID_ADDRESS), cut.getTransmissionAddress());
}

/**
 * \req: [BSW_DoCAN_70], [BSW_DoCAN_78]
 */
TEST(DoCanMessageTransmitterTest, testMultipleFramesAreEmitted)
{
    uint8_t data[]
        = {0x12,
           0x34,
           0x56,
           0x78,
           0x9a,
           0x34,
           0x48,
           0xf1,
           0xfc,
           0x19,
           0x28,
           0x37,
           0x46,
           0x54,
           0x63,
           0x72,
           0x81};
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    TransportMessage message(data, sizeof(data));
    message.setPayloadLength(sizeof(data));
    JobHandle jobHandle(66, 77);
    DoCanMessageTransmitter<DataLinkLayer> cut(
        jobHandle, codec, DataLinkLayer::AddressPairType(0x123, 0x456), message, nullptr, 3U, 7U);
    EXPECT_EQ(TransmitState::INITIALIZED, cut.getState());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(data), cut.getSendData()));
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U, 6U));
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
    EXPECT_EQ(
        TransmitResult(true).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 0, 0, 2));
    EXPECT_EQ(
        true,
        ::etl::equal(::etl::span<uint8_t const>(data + 6U, sizeof(data) - 6U), cut.getSendData()));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U, 7U));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(
        true,
        ::etl::equal(
            ::etl::span<uint8_t const>(data + 13U, sizeof(data) - 13U), cut.getSendData()));
    // ignore unexpected flow control
    EXPECT_EQ(TransmitResult(false), cut.handleFlowControl(FlowStatus::CTS, 0, 0, 2));
    // send on
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U, 4U));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::SUCCESS, cut.getState());
    cut.release();
    EXPECT_EQ(uint32_t(DataLinkLayer::INVALID_ADDRESS), cut.getReceptionAddress());
    EXPECT_EQ(uint32_t(DataLinkLayer::INVALID_ADDRESS), cut.getTransmissionAddress());
}

/**
 * \req: [BSW_DoCAN_70], [BSW_DoCAN_78]
 */
TEST(DoCanMessageTransmitterTest, testMultipleFramesAreEmittedWithEscapeSequence)
{
    uint8_t data[0x1111];
    for (size_t i = 0; i < 0x1111; i++)
    {
        data[i] = i & 0xFF;
    }
    auto span = ::etl::span<uint8_t const>(data);

    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    TransportMessage message(data, span.size());
    message.setPayloadLength(span.size());
    JobHandle jobHandle(66, 77);
    DoCanMessageTransmitter<DataLinkLayer> cut(
        jobHandle, codec, DataLinkLayer::AddressPairType(0x123, 0x456), message, nullptr, 625U, 7U);
    EXPECT_EQ(TransmitState::INITIALIZED, cut.getState());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.start());
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_TRUE(::etl::equal(span, cut.getSendData()));
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(1U, 2U));
    span.advance(2);
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::FLOW_CONTROL, cut.getTimeout());
    EXPECT_EQ(
        TransmitResult(true).setActionSet(storeSeparationTime),
        cut.handleFlowControl(FlowStatus::CTS, 0, 0, 2));
    EXPECT_TRUE(::etl::equal(span, cut.getSendData()));
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(51U, 51 * 7U));
    span.advance(51UL * 7UL);
    EXPECT_EQ(TransmitState::SEND, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_TRUE(::etl::equal(span, cut.getSendData()));
    // ignore unexpected flow control
    EXPECT_EQ(TransmitResult(false), cut.handleFlowControl(FlowStatus::CTS, 0, 0, 2));
    // send on
    EXPECT_EQ(TransmitResult(true), cut.frameSending());
    EXPECT_EQ(TransmitState::WAIT, cut.getState());
    EXPECT_EQ(TransmitTimeout::TX_CALLBACK, cut.getTimeout());
    EXPECT_EQ(TransmitResult(true), cut.framesSent(625 - 1 - 51, span.size()));
    EXPECT_TRUE(cut.isDone());
    EXPECT_EQ(TransmitTimeout::NONE, cut.getTimeout());
    EXPECT_EQ(TransmitState::SUCCESS, cut.getState());
    cut.release();
    EXPECT_EQ(uint32_t(DataLinkLayer::INVALID_ADDRESS), cut.getReceptionAddress());
    EXPECT_EQ(uint32_t(DataLinkLayer::INVALID_ADDRESS), cut.getTransmissionAddress());
}

TEST(DoCanMessageTransmitterTest, testTimers)
{
    uint8_t data[12];
    for (size_t i = 0; i < 12; i++)
    {
        data[i] = i & 0xFF;
    }
    auto span = ::etl::span<uint8_t const>(data);
    DoCanDefaultFrameSizeMapper<uint8_t> const mapper;
    CodecType codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, mapper);
    TransportMessage message(data, span.size());
    message.setPayloadLength(span.size());
    JobHandle jobHandle(66, 77);
    DoCanMessageTransmitter<DataLinkLayer> cut1(
        jobHandle, codec, DataLinkLayer::AddressPairType(0x123, 0x456), message, nullptr, 2U, 7U);
    DoCanMessageTransmitter<DataLinkLayer> cut2(
        jobHandle, codec, DataLinkLayer::AddressPairType(0x321, 0x654), message, nullptr, 2U, 7U);

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
