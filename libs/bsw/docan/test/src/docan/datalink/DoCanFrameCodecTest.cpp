// Copyright 2024 Accenture.

#include "docan/datalink/DoCanFrameCodec.h"

#include "docan/common/DoCanConstants.h"
#include "docan/datalink/DoCanDataLinkLayer.h"
#include "docan/datalink/DoCanDefaultFrameSizeMapper.h"
#include "docan/datalink/DoCanFdFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

#include <etl/memory.h>
#include <etl/span.h>

#include <gmock/gmock.h>

#include <limits>

namespace
{
using namespace docan;

using DataLinkLayerType = DoCanDataLinkLayer<uint32_t, uint16_t, uint8_t, 0xFFFFFFFFU>;

using FrameCodec = DoCanFrameCodec<DataLinkLayerType>;

template<class FrameSize>
class StubSizeMapper
{
public:
    using FrameSizeType = FrameSize;

    bool mapFrameSize(FrameSizeType& size)
    {
        (void)size;
        return true;
    }
};

DoCanFdFrameSizeMapper<uint8_t> const _fdMapper;

/**
 * \req: [BSW_DoCAN_73], [BSW_DoCAN_89], [BSW_DoCAN_99], [BSW_DoCAN_101], [BSW_DoCAN_121]
 */
TEST(DoCanFrameCodecTest, testDecodeFirstFramesWithEscapeSeqAndShortMinFrame)
{
    DoCanFrameCodecConfig<uint8_t> SHORT_FF_CLASSIC
        = {{0U, 8U}, // SF
           {6U, 8U}, // FF
           {0U, 8U}, // CF
           {0U, 8U}, // FC
           0xCCU,    // filler
           1U};      // offset};
    using EscapeDataLinkLayer = DoCanDataLinkLayer<uint32_t, uint32_t, uint8_t, 0xFFFFFFFFU>;
    DoCanFrameCodec<EscapeDataLinkLayer> cut(SHORT_FF_CLASSIC, _fdMapper);
    ::etl::span<uint8_t const> data;
    uint32_t messageSize;
    uint16_t frameCount;
    uint8_t consecutiveFrameDataSize;
    {
        // too short first frame due to escapeSequence
        uint8_t const escapeSeqPayload[] = {0xab, 0x10, 0x00, 0x12, 0x34, 0x56};
        uint8_t const normalPayload[]    = {0xab, 0x11, 0x23, 0xAB, 0xCD, 0xEF};
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_SIZE,
            cut.decodeFirstFrame(
                escapeSeqPayload, messageSize, frameCount, consecutiveFrameDataSize, data));
        EXPECT_EQ(
            CodecResult::OK,
            cut.decodeFirstFrame(
                normalPayload, messageSize, frameCount, consecutiveFrameDataSize, data));
        EXPECT_EQ(0x123, messageSize);
        EXPECT_EQ(73U, frameCount);
        EXPECT_EQ(4U, consecutiveFrameDataSize);
        EXPECT_TRUE(::etl::equal(etl::span<uint8_t const>(normalPayload).subspan(3U), data));
    }
    {
        // Size max(uint_16_t) + 1 using escape sequence MessageType uint32_t
        uint8_t const payload[] = {0xab, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0xA5};
        EXPECT_EQ(
            CodecResult::OK,
            cut.decodeFirstFrame(payload, messageSize, frameCount, consecutiveFrameDataSize, data));

        EXPECT_EQ(0x10000, messageSize);
        EXPECT_EQ(10923U, frameCount);
        EXPECT_EQ(6U, consecutiveFrameDataSize);
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(7U), data));
    }
    {
        // Size ( ( max(uint16_t) frameCount * 6 cFrameDataSize ) - 1 ) = 0x5FFF9 using escape
        // sequence MessageType uint32_t
        uint8_t const payload[] = {0xab, 0x10, 0x00, 0x00, 0x05, 0xFF, 0xF9, 0xA5};
        EXPECT_EQ(
            CodecResult::OK,
            cut.decodeFirstFrame(payload, messageSize, frameCount, consecutiveFrameDataSize, data));

        EXPECT_EQ(0x5FFF9, messageSize);
        EXPECT_EQ(65535U, frameCount);
        EXPECT_EQ(6U, consecutiveFrameDataSize);
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(7U), data));
    }
    {
        // Size max(uint_32_t) using escape sequence MessageType uint32_t but too short
        // consecutiveFrameDataSize to fit the number of frames to FrameIndexType Would have been
        uint8_t const payload[] = {0xab, 0x10, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
        EXPECT_EQ(
            CodecResult::INVALID_MESSAGE_SIZE,
            cut.decodeFirstFrame(payload, messageSize, frameCount, consecutiveFrameDataSize, data));
    }
    {
        // getEncodedFrameCount maximum message size with escaped sequence and uint16 frame count
        EXPECT_EQ(
            CodecResult::OK,
            cut.getEncodedFrameCount(
                (::std::numeric_limits<uint16_t>::max() * 6) - 1,
                frameCount,
                consecutiveFrameDataSize));
        EXPECT_EQ(::std::numeric_limits<uint16_t>::max(), frameCount);
        EXPECT_EQ(6U, consecutiveFrameDataSize);
    }
    {
        // getEncodedFrameCount Bigger than maximum message size with escaped sequence and uint16
        // frame count
        EXPECT_EQ(
            CodecResult::INVALID_MESSAGE_SIZE,
            cut.getEncodedFrameCount(
                ::std::numeric_limits<uint32_t>::max(), frameCount, consecutiveFrameDataSize));
    }
}

/**
 * \req: [BSW_DoCAN_73], [BSW_DoCAN_89], [BSW_DoCAN_99], [BSW_DoCAN_101], [BSW_DoCAN_121]
 */
TEST(DoCanFrameCodecTest, testDecodeFirstFramesWithEscapeSeqAndUint32FrameIndexType)
{
    using EscapeDataLinkLayer
        = DoCanDataLinkLayer<uint32_t, uint32_t, uint8_t, 0xFFFFFFFFU, uint32_t>;
    DoCanFrameCodec<EscapeDataLinkLayer> cut(
        DoCanFrameCodecConfigPresets::PADDED_CLASSIC, _fdMapper);
    ::etl::span<uint8_t const> data;
    uint32_t messageSize;
    uint32_t frameCount;
    uint8_t consecutiveFrameDataSize;
    {
        uint8_t const payload[] = {0x10, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xA5, 0xA5};
        EXPECT_EQ(
            CodecResult::OK,
            cut.decodeFirstFrame(payload, messageSize, frameCount, consecutiveFrameDataSize, data));
        EXPECT_EQ(0xFFFFFFFF, messageSize);
        EXPECT_EQ(7, consecutiveFrameDataSize);
        EXPECT_EQ(613566757, frameCount);
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(6U), data));
    }
    {
        // getEncodedFrameCount maximum message size with escaped sequence and uint32
        EXPECT_EQ(
            CodecResult::OK,
            cut.getEncodedFrameCount(
                ::std::numeric_limits<uint32_t>::max(), frameCount, consecutiveFrameDataSize));
        EXPECT_EQ(613566757, frameCount);
        EXPECT_EQ(7, consecutiveFrameDataSize);
    }
}

/**
 * \req: [BSW_DoCAN_73], [BSW_DoCAN_89], [BSW_DoCAN_99], [BSW_DoCAN_101], [BSW_DoCAN_121]
 */
TEST(DoCanFrameCodecTest, testDecodeFirstFramesWithLongFramesEscapeSeqAndUint32FrameIndexType)
{
    using EscapeDataLinkLayer
        = DoCanDataLinkLayer<uint32_t, uint32_t, uint8_t, 0xFFFFFFFFU, uint32_t>;
    DoCanFrameCodec<EscapeDataLinkLayer> cut(DoCanFrameCodecConfigPresets::PADDED_FD, _fdMapper);
    ::etl::span<uint8_t const> data;
    uint32_t messageSize;
    uint32_t frameCount;
    uint8_t consecutiveFrameDataSize;
    {
        uint8_t payload[64] = {0x10, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
        for (size_t i = 6; i < sizeof(payload); i++)
        {
            payload[i] = i & 0xFF;
        }

        EXPECT_EQ(
            CodecResult::OK,
            cut.decodeFirstFrame(payload, messageSize, frameCount, consecutiveFrameDataSize, data));
        EXPECT_EQ(0xFFFFFFFF, messageSize);
        EXPECT_EQ(63, consecutiveFrameDataSize);
        EXPECT_EQ(68174085, frameCount);
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(6U), data));
    }
    {
        // getEncodedFrameCount maximum message size with escaped sequence and uint32
        EXPECT_EQ(
            CodecResult::OK,
            cut.getEncodedFrameCount(
                ::std::numeric_limits<uint32_t>::max(), frameCount, consecutiveFrameDataSize));
        EXPECT_EQ(68174085, frameCount);
        EXPECT_EQ(63, consecutiveFrameDataSize);
    }
}

/*
 * If a first frame doesn't fit into a short single frame, but does fit into a long single frame
 * (and the minimum length of a first frame is allowed to be less than the maximum length of a
 * first frame) then decodeFirstFrame will return an invalid message size
 */
TEST(DoCanFrameCodecTest, firstFrameFitsIntoLongSingleFrame)
{
    // Custom frame codec config to allow minimum first frame size to be less than maximum first
    // frame size
    DoCanFrameCodecConfig<uint8_t> const CUSTOM_FD
        = {{0U, 64U}, // SF
           {0U, 64U}, // FF
           {0U, 64U}, // CF
           {0U, 64U}, // FC
           0xCCU,     // filler
           1U};       // offset
    FrameCodec cut(CUSTOM_FD, _fdMapper);
    ::etl::span<uint8_t const> data;
    uint16_t messageSize;
    uint16_t frameCount;
    uint8_t consecutiveFrameDataSize;
    {
        // too long for a short single frame, but does fit into a long single frame
        uint8_t const payload[] = {0xAB, 0x10, 0x0B, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};
        EXPECT_EQ(
            CodecResult::INVALID_MESSAGE_SIZE,
            cut.decodeFirstFrame(payload, messageSize, frameCount, consecutiveFrameDataSize, data));
    }
}

/*
 * If the configured single frame max size is smaller than the configured offset + 2, an assert
 * shall be triggered in the DoCanFrameCodec. This ensures that we can never produce a
 * negative/wrapped-around value when trying to calculate if a specific message size can fit into a
 * long single frame
 */
TEST(DoCanFrameCodecTest, singleFrameMaxSizeSmallerThanOffset)
{
    // Custom frame codec config to make the single frame max (2) too close to the offset (1), which
    // will trigger an assert when "fitsIntoLongSingleFrame" is called
    DoCanFrameCodecConfig<uint8_t> const CUSTOM_FD
        = {{0U, 2U},  // SF
           {0U, 64U}, // FF
           {0U, 64U}, // CF
           {0U, 64U}, // FC
           0xCCU,     // filler
           1U};       // offset
    FrameCodec cut(CUSTOM_FD, _fdMapper);
    ::etl::span<uint8_t const> data;
    uint16_t messageSize;
    uint16_t frameCount;
    uint8_t consecutiveFrameDataSize;
    {
        uint8_t const payload[] = {0xAB, 0x10, 0x0B, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};
        ASSERT_THROW(
            cut.decodeFirstFrame(payload, messageSize, frameCount, consecutiveFrameDataSize, data),
            ::etl::exception);
    }
}

/*
 * If the configured consecutive frame max size is smaller than the configured offset + 2, an assert
 * shall be triggered in the DoCanFrameCodec. This ensures that we can never produce a
 * negative/wrapped-around value when trying to calculate if a specific message size can fit into a
 * consecutive frame
 */
TEST(DoCanFrameCodecTest, consecutiveFrameMaxSizeSmallerThanOffset)
{
    using DataLinkLayerType = DoCanDataLinkLayer<uint32_t, uint16_t, uint8_t, 0xFFFFFFFFU>;
    // Custom frame codec config to make the single frame max (2) too close to the offset (1), which
    // will trigger an assert when "fitsIntoLongSingleFrame" is called
    DoCanFrameCodecConfig<uint8_t> const CUSTOM_FD
        = {{0U, 64U}, // SF
           {0U, 64U}, // FF
           {0U, 2U},  // CF
           {0U, 64U}, // FC
           0xCCU,     // filler
           1U};       // offset
    FrameCodec cut(CUSTOM_FD, _fdMapper);
    {
        DataLinkLayerType::FrameIndexType frameIdx = 0;
        DataLinkLayerType::FrameSizeType frameSize = 0;
        ASSERT_THROW(cut.getEncodedFrameCount(0, frameIdx, frameSize), ::etl::exception);
    }
}

/**
 * \req: [BSW_DoCAN_111]
 */
TEST(DoCanFrameCodecTest, testDecodeFlowControlFramesWithFullSize)
{
    DoCanFrameCodecConfig<uint8_t> config = DoCanFrameCodecConfigPresets::PADDED_CLASSIC;
    config._offset                        = 1U;
    FrameCodec cut(config, _fdMapper);
    FlowStatus flowStatus;
    uint8_t blockSize;
    uint8_t encodedMinSeparationTime;
    {
        // valid flow control frame
        uint8_t const payload[8] = {0xab, 0x32, 0x12, 0x34, 0xaa, 0xaa, 0xaa, 0xaa};
        EXPECT_EQ(
            CodecResult::OK,
            cut.decodeFlowControlFrame(payload, flowStatus, blockSize, encodedMinSeparationTime));
        EXPECT_EQ(FlowStatus::OVFLW, flowStatus);
        EXPECT_EQ(0x12U, blockSize);
        EXPECT_EQ(0x34U, encodedMinSeparationTime);
    }
    {
        // too short flow control frame
        uint8_t const payload[4] = {0xab, 0x34, 0x12, 0x34};
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_SIZE,
            cut.decodeFlowControlFrame(payload, flowStatus, blockSize, encodedMinSeparationTime));
    }
    {
        // too long flow control frame
        uint8_t const payload[9] = {0xab, 0x32, 0x12, 0x34, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_SIZE,
            cut.decodeFlowControlFrame(payload, flowStatus, blockSize, encodedMinSeparationTime));
    }
}

/**
 * \req: [BSW_DoCAN_87]
 */
TEST(DoCanFrameCodecTest, testDecodeFramesWithEnforcedPadding)
{
    DoCanFrameCodecConfig<uint8_t> const config
        = {{6U, 6U}, // SF
           {6U, 6U}, // FF
           {6U, 6U}, // CF
           {6U, 6U}, // FC
           0xCCU,    // filler
           1U};      // offset
    FrameCodec cut(config, _fdMapper);
    ::etl::span<uint8_t const> data;
    {
        uint16_t messageSize;
        {
            // valid minimum single frame
            uint8_t const payload[] = {0xab, 0x01, 0x12, 0xaa, 0xaa, 0xaa};
            EXPECT_EQ(CodecResult::OK, cut.decodeSingleFrame(payload, messageSize, data));
            EXPECT_EQ(1U, messageSize);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(2U, 1U), data));
        }
        {
            // valid maximum single frame
            uint8_t const payload[] = {0xab, 0x04, 0x12, 0x34, 0x56, 0x78};
            EXPECT_EQ(CodecResult::OK, cut.decodeSingleFrame(payload, messageSize, data));
            EXPECT_EQ(4U, messageSize);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(2U), data));
        }
        {
            // too short single frame
            uint8_t const payload[] = {0xab, 0x06, 0x12, 0x34, 0x56};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE, cut.decodeSingleFrame(payload, messageSize, data));
        }
        {
            // too long single frame
            uint8_t const payload[] = {0xab, 0x06, 0x12, 0x34, 0x56, 0x78, 0x9a};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE, cut.decodeSingleFrame(payload, messageSize, data));
        }
    }
    // first frames
    {
        uint16_t messageSize;
        uint16_t frameCount;
        uint8_t consecutiveFrameDataSize;
        {
            // valid minimum first frame
            uint8_t const payload[] = {0xab, 0x10, 0x05, 0x12, 0x34, 0x56};
            EXPECT_EQ(
                CodecResult::OK,
                cut.decodeFirstFrame(
                    payload, messageSize, frameCount, consecutiveFrameDataSize, data));
            EXPECT_EQ(5U, messageSize);
            EXPECT_EQ(2U, frameCount);
            EXPECT_EQ(4U, consecutiveFrameDataSize);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(3U), data));
        }
        {
            // too short first frame
            uint8_t const payload[] = {0xab, 0x10, 0x07, 0x12, 0x34};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE, cut.decodeSingleFrame(payload, messageSize, data));
        }
        {
            // too long first frame
            uint8_t const payload[] = {0xab, 0x10, 0x07, 0x12, 0x34, 0x56, 0x78};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE, cut.decodeSingleFrame(payload, messageSize, data));
        }
    }
    // consecutive frames
    {
        uint8_t sequenceNumber;
        {
            // valid consecutive frame
            uint8_t const payload[] = {0xab, 0x23, 0x12, 0xaa, 0xaa, 0xaa};
            EXPECT_EQ(CodecResult::OK, cut.decodeConsecutiveFrame(payload, sequenceNumber, data));
            EXPECT_EQ(3U, sequenceNumber);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(2U), data));
        }
        {
            // too short consecutive frame
            uint8_t const payload[] = {0xab, 0x23, 0x12, 0xaa, 0xaa};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeConsecutiveFrame(payload, sequenceNumber, data));
        }
        {
            // too long first frame
            uint8_t const payload[] = {0xab, 0x23, 0x12, 0xaa, 0xaa, 0xaa, 0xaa};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeConsecutiveFrame(payload, sequenceNumber, data));
        }
    }
    // flow control frames
    {
        FlowStatus flowStatus;
        uint8_t blockSize;
        uint8_t encodedMinSeparationTime;
        {
            // valid flow control frame
            uint8_t const payload[] = {0xab, 0x32, 0x12, 0x34, 0xaa, 0xaa};
            EXPECT_EQ(
                CodecResult::OK,
                cut.decodeFlowControlFrame(
                    payload, flowStatus, blockSize, encodedMinSeparationTime));
            EXPECT_EQ(FlowStatus::OVFLW, flowStatus);
            EXPECT_EQ(0x12U, blockSize);
            EXPECT_EQ(0x34U, encodedMinSeparationTime);
        }
        {
            // too short flow control frame
            uint8_t const payload[] = {0xab, 0x32, 0x12, 0x34, 0xaa};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeFlowControlFrame(
                    payload, flowStatus, blockSize, encodedMinSeparationTime));
        }
        {
            // too long flow control frame
            uint8_t const payload[] = {0xab, 0x32, 0x12, 0x34, 0xaa, 0xaa, 0xaa};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeFlowControlFrame(
                    payload, flowStatus, blockSize, encodedMinSeparationTime));
        }
    }
}

/**
 * \req: [BSW_DoCAN_87], [BSW_DoCAN_176], [BSW_DoCAN_177]
 */
TEST(DoCanFrameCodecTest, testDecodeShortAndLongFramesWithEnforcedPaddingUnmapped)
{
    DoCanFrameCodecConfig<uint8_t> const config
        = {{6U, 14U},  // SF
           {14U, 14U}, // FF
           {6U, 14U},  // CF
           {6U, 14U},  // FC
           0xCCU,      // filler
           1U};        // offset
    FrameCodec cut(config, _fdMapper);
    ::etl::span<uint8_t const> data;
    {
        uint16_t messageSize;
        {
            // valid minimum short single frame
            uint8_t const payload[] = {0xab, 0x01, 0x12, 0xaa, 0xaa, 0xaa};
            EXPECT_EQ(CodecResult::OK, cut.decodeSingleFrame(payload, messageSize, data));
            EXPECT_EQ(1U, messageSize);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(2U, 1U), data));
        }
        {
            // valid maximum short single frame
            uint8_t const payload[] = {0xab, 0x04, 0x12, 0x34, 0x56, 0x78};
            EXPECT_EQ(CodecResult::OK, cut.decodeSingleFrame(payload, messageSize, data));
            EXPECT_EQ(4U, messageSize);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(2U), data));
        }
        {
            // valid minimum long single frame
            uint8_t const payload[14] = {0xab, 0x00, 0x05, 0x12, 0xaa, 0xaa, 0xaa, 0xaa};
            EXPECT_EQ(CodecResult::OK, cut.decodeSingleFrame(payload, messageSize, data));
            EXPECT_EQ(5U, messageSize);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(3U, 5U), data));
        }
        {
            // valid maximum long single frame
            uint8_t const payload[14] = {0xab, 0x00, 0x0b, 0x04, 0x12, 0x34, 0x56, 0x78};
            EXPECT_EQ(CodecResult::OK, cut.decodeSingleFrame(payload, messageSize, data));
            EXPECT_EQ(11U, messageSize);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(3U), data));
        }
        {
            // too short short single frame
            uint8_t const payload[] = {0xab, 0x06, 0x12, 0x34, 0x56};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE, cut.decodeSingleFrame(payload, messageSize, data));
        }
        {
            // too long short single frame
            uint8_t const payload[] = {0xab, 0x06, 0x12, 0x34, 0x56, 0x78, 0x9a};
            EXPECT_EQ(
                CodecResult::INVALID_MESSAGE_SIZE,
                cut.decodeSingleFrame(payload, messageSize, data));
        }
        {
            // too short long single frame
            uint8_t const payload[13] = {0xab, 0x06, 0x12, 0x34, 0x56};
            EXPECT_EQ(
                CodecResult::INVALID_MESSAGE_SIZE,
                cut.decodeSingleFrame(payload, messageSize, data));
        }
        {
            // too long long single frame
            uint8_t const payload[15] = {0xab, 0x06, 0x12, 0x34, 0x56, 0x78, 0x9a};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE, cut.decodeSingleFrame(payload, messageSize, data));
        }
    }
    // first frames
    {
        uint16_t messageSize;
        uint16_t frameCount;
        uint8_t consecutiveFrameDataSize;
        {
            // valid minimum short first frame
            uint8_t const payload[14U] = {0xab, 0x10, 12, 0x12, 0x34, 0x56};
            EXPECT_EQ(
                CodecResult::OK,
                cut.decodeFirstFrame(
                    payload, messageSize, frameCount, consecutiveFrameDataSize, data));
            EXPECT_EQ(12U, messageSize);
            EXPECT_EQ(2U, frameCount);
            EXPECT_EQ(12U, consecutiveFrameDataSize);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(3U), data));
        }
        {
            // valid minimum long first frame
            uint8_t const payload[14] = {0xab, 0x10, 0x0d, 0x12, 0x34, 0x56};
            EXPECT_EQ(
                CodecResult::OK,
                cut.decodeFirstFrame(
                    payload, messageSize, frameCount, consecutiveFrameDataSize, data));
            EXPECT_EQ(13U, messageSize);
            EXPECT_EQ(2U, frameCount);
            EXPECT_EQ(12U, consecutiveFrameDataSize);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(3U), data));
        }
        {
            // too short short first frame
            uint8_t const payload[] = {0xab, 0x10, 0x07, 0x12, 0x34};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeFirstFrame(
                    payload, messageSize, frameCount, consecutiveFrameDataSize, data));
        }
        {
            // too long short first frame
            uint8_t const payload[] = {0xab, 0x10, 0x07, 0x12, 0x34, 0x56, 0x78};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeFirstFrame(
                    payload, messageSize, frameCount, consecutiveFrameDataSize, data));
        }
        {
            // too short long first frame
            uint8_t const payload[13] = {0xab, 0x10, 0x07, 0x12, 0x34};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeFirstFrame(
                    payload, messageSize, frameCount, consecutiveFrameDataSize, data));
        }
        {
            // too long short first frame
            uint8_t const payload[15] = {0xab, 0x10, 0x07, 0x12, 0x34, 0x56, 0x78};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeFirstFrame(
                    payload, messageSize, frameCount, consecutiveFrameDataSize, data));
        }
    }
    // consecutive frames
    {
        uint8_t sequenceNumber;
        {
            // valid short consecutive frame
            uint8_t const payload[] = {0xab, 0x23, 0x12, 0xaa, 0xaa, 0xaa};
            EXPECT_EQ(CodecResult::OK, cut.decodeConsecutiveFrame(payload, sequenceNumber, data));
            EXPECT_EQ(3U, sequenceNumber);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(2U), data));
        }
        {
            // valid long consecutive frame
            uint8_t const payload[14] = {0xab, 0x23, 0x12, 0xaa, 0xaa, 0xaa};
            EXPECT_EQ(CodecResult::OK, cut.decodeConsecutiveFrame(payload, sequenceNumber, data));
            EXPECT_EQ(3U, sequenceNumber);
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(payload).subspan(2U), data));
        }
        {
            // too short short consecutive frame
            uint8_t const payload[] = {0xab, 0x23, 0x12, 0xaa, 0xaa};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeConsecutiveFrame(payload, sequenceNumber, data));
        }
        {
            // too long consecutive frame
            uint8_t const payload[15] = {0xab, 0x23, 0x12, 0xaa, 0xaa};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeConsecutiveFrame(payload, sequenceNumber, data));
        }
    }
    // flow control frames
    {
        FlowStatus flowStatus;
        uint8_t blockSize;
        uint8_t encodedMinSeparationTime;
        {
            // valid short flow control frame
            uint8_t const payload[] = {0xab, 0x32, 0x12, 0x34, 0xaa, 0xaa};
            EXPECT_EQ(
                CodecResult::OK,
                cut.decodeFlowControlFrame(
                    payload, flowStatus, blockSize, encodedMinSeparationTime));
            EXPECT_EQ(FlowStatus::OVFLW, flowStatus);
            EXPECT_EQ(0x12U, blockSize);
            EXPECT_EQ(0x34U, encodedMinSeparationTime);
        }
        {
            // valid long flow control frame
            uint8_t const payload[14] = {0xab, 0x32, 0x12, 0x34, 0xaa, 0xaa};
            EXPECT_EQ(
                CodecResult::OK,
                cut.decodeFlowControlFrame(
                    payload, flowStatus, blockSize, encodedMinSeparationTime));
            EXPECT_EQ(FlowStatus::OVFLW, flowStatus);
            EXPECT_EQ(0x12U, blockSize);
            EXPECT_EQ(0x34U, encodedMinSeparationTime);
        }
        {
            // too short flow control frame
            uint8_t const payload[] = {0xab, 0x32, 0x12, 0x34, 0xaa};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeFlowControlFrame(
                    payload, flowStatus, blockSize, encodedMinSeparationTime));
        }
        {
            // too long flow control frame
            uint8_t const payload[15] = {0xab, 0x32, 0x12, 0x34, 0xaa, 0xaa, 0xaa};
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_SIZE,
                cut.decodeFlowControlFrame(
                    payload, flowStatus, blockSize, encodedMinSeparationTime));
        }
    }
}

TEST(DoCanFrameCodecTest, testEncodeDataFrame)
{
    DoCanFrameCodecConfig<uint8_t> const config
        = {{3U, 8U}, // SF
           {8U, 8U}, // FF
           {3U, 8U}, // CF
           {3U, 8U}, // FC
           0xCCU,    // filler
           1U};      // offset
    FrameCodec cut(config, _fdMapper);
    uint8_t frame[10U];
    frame[0] = 0xdcU;
    frame[9] = 0xefU;
    uint8_t consumedDataSize;
    {
        // write a short single frame
        uint8_t const data[]     = {0x12, 0x34, 0x78};
        uint8_t const expected[] = {0xdc, 0x03, 0x12, 0x34, 0x78};
        ::etl::span<uint8_t> payload(frame);
        EXPECT_EQ(CodecResult::OK, cut.encodeDataFrame(payload, data, 0U, 0U, consumedDataSize));
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
        EXPECT_EQ(sizeof(data), consumedDataSize);
    }
    {
        // write a segmented message
        uint8_t const message[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};
        ::etl::span<uint8_t const> data(message);
        {
            // first frame
            uint8_t const expected[] = {0xdc, 0x10, 0x07, 0x12, 0x34, 0x56, 0x78, 0x9a};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 0U, 6U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(5U, consumedDataSize);
            data.advance(5U);
        }
        {
            // consecutive frame
            uint8_t const expected[] = {0xdc, 0x21, 0xbc, 0xde};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 1U, 6U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(2U, consumedDataSize);
        }
    }
    {
        // write a segmented message with minimum segment size
        uint8_t const message[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};
        ::etl::span<uint8_t const> data(message);
        {
            // first frame
            uint8_t const expected[] = {0xdc, 0x10, 0x07, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 0U, 1U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(0U, consumedDataSize);
        }
        {
            // consecutive frame 1
            uint8_t const expected[] = {0xdc, 0x21, 0x12};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 1U, 1U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(1U, consumedDataSize);
            data.advance(1U);
        }
        {
            // consecutive frame 2
            uint8_t const expected[] = {0xdc, 0x22, 0x34};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 2U, 1U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(1U, consumedDataSize);
            data.advance(1U);
        }
        {
            // consecutive frame 3
            uint8_t const expected[] = {0xdc, 0x23, 0x56};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 3U, 1U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(1U, consumedDataSize);
            data.advance(1U);
        }
        {
            // consecutive frame 4
            uint8_t const expected[] = {0xdc, 0x24, 0x78};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 4U, 1U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(1U, consumedDataSize);
            data.advance(1U);
        }
        {
            // consecutive frame 5
            uint8_t const expected[] = {0xdc, 0x25, 0x9a};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 5U, 1U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(1U, consumedDataSize);
            data.advance(1U);
        }
        {
            // consecutive frame 6
            uint8_t const expected[] = {0xdc, 0x26, 0xbc};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 6U, 1U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(1U, consumedDataSize);
            data.advance(1U);
        }
        {
            // consecutive frame 7
            uint8_t const expected[] = {0xdc, 0x27, 0xde};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 7U, 1U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(1U, consumedDataSize);
            data.advance(1U);
        }
    }
    {
        // write a segmented message with escape sequence
        constexpr auto MESSAGE_SIZE                = 4321;
        constexpr auto CONSECUTIVE_FRAME_DATA_SIZE = 6;
        constexpr auto FRAMES = ((MESSAGE_SIZE - 1) / CONSECUTIVE_FRAME_DATA_SIZE);
        uint8_t message[MESSAGE_SIZE];
        for (size_t i = 0; i < MESSAGE_SIZE; i++)
        {
            message[i] = (i + 0xA5) & 0xFF;
        }
        ::etl::span<uint8_t const> data(message);
        {
            // first frame, with escape sequence
            uint8_t const expected[] = {0xdc, 0x10, 0x00, 0x00, 0x00, 0x10, 0xE1, data[0]};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK,
                cut.encodeDataFrame(
                    payload, data, 0U, CONSECUTIVE_FRAME_DATA_SIZE, consumedDataSize));
            EXPECT_THAT(payload, ::testing::ElementsAreArray(expected));
            EXPECT_EQ(1U, consumedDataSize);
            data.advance(consumedDataSize);
        }
        size_t i = 0;
        for (; i < FRAMES; i++)
        {
            // consecutive frame
            uint8_t const expected[]
                = {0xdc,
                   static_cast<uint8_t>(
                       0x20 + ((0x01 + i) % 0x10)), // Sequence numbers wraps around, 0-15
                   data[0],
                   data[1],
                   data[2],
                   data[3],
                   data[4],
                   data[5]};
            ::etl::span<uint8_t> payload(frame);
            auto result = cut.encodeDataFrame(
                payload, data, i + 1, CONSECUTIVE_FRAME_DATA_SIZE, consumedDataSize);
            ASSERT_EQ(CodecResult::OK, result);
            EXPECT_THAT(payload, ::testing::ElementsAreArray(expected));
            EXPECT_EQ(6U, consumedDataSize);
            data.advance(consumedDataSize);
        }
        EXPECT_EQ(i, FRAMES);
        EXPECT_EQ(data.size(), 0);
    }
    {
        // write a segmented message with escape sequence but bigger size than MessageSize allows
        constexpr uint64_t MESSAGE_SIZE
            = ::std::numeric_limits<FrameCodec::MessageSizeType>::max() + 1;
        constexpr auto CONSECUTIVE_FRAME_DATA_SIZE = 6;
        uint8_t message[MESSAGE_SIZE];
        for (size_t i = 0; i < MESSAGE_SIZE; i++)
        {
            message[i] = (i + 0xA5) & 0xFF;
        }
        ::etl::span<uint8_t const> data(message);
        {
            // first frame, with escape sequence
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::INVALID_FRAME_INDEX,
                cut.encodeDataFrame(
                    payload, data, 0U, CONSECUTIVE_FRAME_DATA_SIZE, consumedDataSize));
        }
    }
    {
        // write a first frame of a segmented message without consecutive frame size
        uint8_t const data[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};
        ::etl::span<uint8_t> payload(frame);
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_INDEX,
            cut.encodeDataFrame(payload, data, 0U, 0U, consumedDataSize));
    }
    {
        // write a single frame with too short payload
        uint8_t const data[] = {0x12, 0x34, 0x78};
        auto payload         = ::etl::span<uint8_t>(frame).first(4U);
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_SIZE,
            cut.encodeDataFrame(payload, data, 0U, 0U, consumedDataSize));
    }
    {
        // write a single frame with no more payload
        ::etl::span<uint8_t> payload(frame);
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_INDEX,
            cut.encodeDataFrame(payload, {}, 0U, 0U, consumedDataSize));
    }
    {
        // write consecutive frame without a consecutive frame data size set
        uint8_t const data[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};
        ::etl::span<uint8_t> payload(frame);
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_INDEX,
            cut.encodeDataFrame(payload, data, 1U, 0U, consumedDataSize));
    }
    EXPECT_EQ(0xefU, frame[9]);
}

/*
 * If one of the config minimum parameters is larger than the max allowable CAN frame size (64 bytes
 * for CAN FD), then calling "adjustFrame" internal to the frame codec will fail and return an
 * invalid frame size error
 */
TEST(DoCanFrameCodecTest, codecConfigMinimumLargerThanMaxCanFrameSize)
{
    // Custom frame codec config to allow minimum first frame size to be less than maximum first
    // frame size
    DoCanFrameCodecConfig<uint8_t> const CUSTOM_FD
        = {{0U, 64U},  // SF
           {0U, 64U},  // FF
           {0U, 64U},  // CF
           {65U, 65U}, // FC
           0xCCU,      // filler
           1U};        // offset
    FrameCodec cut(CUSTOM_FD, _fdMapper);
    uint8_t frame[10U];
    frame[0] = 0xdcU;
    frame[9] = 0xefU;
    {
        // write a simple flow control frame
        ::etl::span<uint8_t> payload(frame);
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_SIZE,
            cut.encodeFlowControlFrame(payload, FlowStatus::CTS, 0x13, 0x25));
    }
}

/**
 * \req: [BSW_DoCAN_180], [BSW_DoCAN_181]
 */
TEST(DoCanFrameCodecTest, testEncodeFramesWithEnforcedPadding)
{
    DoCanFrameCodecConfig<uint8_t> const config
        = {{8U, 8U}, // SF
           {8U, 8U}, // FF
           {8U, 8U}, // CF
           {8U, 8U}, // FC
           0x91U,    // filler
           1U};      // offset
    FrameCodec cut(config, _fdMapper);
    uint8_t frame[9U];
    frame[0] = 0xdcU;
    frame[8] = 0xefU;
    uint8_t consumedDataSize;
    {
        // write a short single frame
        uint8_t const data[]     = {0x12, 0x34, 0x78};
        uint8_t const expected[] = {0xdc, 0x03, 0x12, 0x34, 0x78, 0x91, 0x91, 0x91};
        ::etl::span<uint8_t> payload(frame);
        EXPECT_EQ(CodecResult::OK, cut.encodeDataFrame(payload, data, 0U, 0U, consumedDataSize));
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
        EXPECT_EQ(sizeof(data), consumedDataSize);
    }
    {
        // write a segmented message
        uint8_t const message[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};
        ::etl::span<uint8_t const> data(message);
        {
            // first frame
            uint8_t const expected[] = {0xdc, 0x10, 0x07, 0x12, 0x34, 0x56, 0x78, 0x9a};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 0U, 6U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(5U, consumedDataSize);
            data.advance(5U);
        }
        {
            // consecutive frame
            uint8_t const expected[] = {0xdc, 0x21, 0xbc, 0xde, 0x91, 0x91, 0x91, 0x91};
            ::etl::span<uint8_t> payload(frame);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 1U, 6U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(2U, consumedDataSize);
        }
    }
    {
        // write a first frame that is longer than expected
        uint8_t const message[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};
        ::etl::span<uint8_t const> data(message);
        uint8_t longerFrame[12];
        longerFrame[0] = 0xdcU;
        uint8_t const expected[]
            = {0xdc, 0x10, 0x07, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0x91, 0x91, 0x91};
        ::etl::span<uint8_t> payload(longerFrame);
        EXPECT_EQ(CodecResult::OK, cut.encodeDataFrame(payload, data, 0U, 7U, consumedDataSize));
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
        EXPECT_EQ(6U, consumedDataSize);
        EXPECT_EQ(0x91U, longerFrame[9]);
    }
    {
        // write a simple flow control frame
        uint8_t const expected[] = {0xdc, 0x30, 0x13, 0x25, 0x91, 0x91, 0x91, 0x91};
        ::etl::span<uint8_t> payload(frame);
        EXPECT_EQ(
            CodecResult::OK, cut.encodeFlowControlFrame(payload, FlowStatus::CTS, 0x13, 0x25));
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
    }
    EXPECT_EQ(0xefU, frame[8]);
}

/**
 * \req: [BSW_DoCAN_180], [BSW_DoCAN_181]
 */
TEST(DoCanFrameCodecTest, testEncodeShortAndLongFramesWithEnforcedPadding)
{
    DoCanFrameCodecConfig<uint8_t> const config
        = {{8U, 20U},  // SF
           {20U, 20U}, // FF
           {8U, 20U},  // CF
           {8U, 20U},  // FC
           0x91U,      // filler
           1U};        // offset
    FrameCodec cut(config, _fdMapper);
    uint8_t frame[21];
    frame[0]  = 0xdcU;
    frame[20] = 0xefU;
    uint8_t consumedDataSize;
    {
        // write a short single frame
        uint8_t const data[]         = {0x12, 0x34, 0x78};
        uint8_t const expected[]     = {0xdc, 0x03, 0x12, 0x34, 0x78, 0x91, 0x91, 0x91};
        ::etl::span<uint8_t> payload = frame;
        payload                      = payload.first(20U);
        EXPECT_EQ(CodecResult::OK, cut.encodeDataFrame(payload, data, 0U, 0U, consumedDataSize));
        EXPECT_THAT(expected, ::testing::ElementsAreArray(payload));
        EXPECT_EQ(sizeof(data), consumedDataSize);
    }
    {
        // write a minimum long single frame
        uint8_t const data[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};
        uint8_t const expected[]
            = {0xdc, 0x00, 0x07, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0x91, 0x91};
        ::etl::span<uint8_t> payload = frame;
        payload                      = payload.first(12U);
        EXPECT_EQ(CodecResult::OK, cut.encodeDataFrame(payload, data, 0U, 0U, consumedDataSize));
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
        EXPECT_EQ(sizeof(data), consumedDataSize);
    }
    {
        // write a minimum size segmented message
        uint8_t const message[]
            = {0x12,
               0x34,
               0x56,
               0x78,
               0x9a,
               0xbc,
               0xde,
               0xf0,
               0x13,
               0x24,
               0x35,
               0x46,
               0x57,
               0x68,
               0x79,
               0x8a,
               0xac,
               0x11};
        ::etl::span<uint8_t const> data(message);
        {
            // first frame
            uint8_t const expected[] = {0xdc, 0x10, 18,   0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde,
                                        0xf0, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79, 0x91, 0x91};
            ::etl::span<uint8_t> payload = frame;
            payload                      = payload.first(20U);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 0U, 16U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(15U, consumedDataSize);
            data.advance(15U);
        }
        {
            // consecutive frame
            uint8_t const expected[]     = {0xdc, 0x21, 0x8a, 0xac, 0x11, 0x91, 0x91, 0x91};
            ::etl::span<uint8_t> payload = frame;
            payload                      = payload.first(8U);
            EXPECT_EQ(
                CodecResult::OK, cut.encodeDataFrame(payload, data, 1U, 16U, consumedDataSize));
            EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
            EXPECT_EQ(3U, consumedDataSize);
        }
    }
    {
        // write a simple flow control frame
        uint8_t const expected[]     = {0xdc, 0x30, 0x13, 0x25, 0x91, 0x91, 0x91, 0x91};
        ::etl::span<uint8_t> payload = frame;
        payload                      = payload.first(20U);
        EXPECT_EQ(
            CodecResult::OK, cut.encodeFlowControlFrame(payload, FlowStatus::CTS, 0x13, 0x25));
        EXPECT_TRUE(::etl::equal(::etl::span<uint8_t const>(expected), payload));
    }
    {
        // write a short single frame with too short payload
        uint8_t const data[] = {0x12, 0x34, 0x78};
        auto payload         = ::etl::span<uint8_t>(frame).first(7U);
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_SIZE,
            cut.encodeDataFrame(payload, data, 0U, 0U, consumedDataSize));
    }
    EXPECT_EQ(0xefU, frame[20]);
}

} // anonymous namespace
