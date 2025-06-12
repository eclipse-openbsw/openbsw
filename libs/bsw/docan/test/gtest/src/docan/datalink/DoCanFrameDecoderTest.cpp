// Copyright 2024 Accenture.

#include "docan/datalink/DoCanFrameDecoder.h"

#include "docan/common/DoCanConnection.h"
#include "docan/common/DoCanConstants.h"
#include "docan/datalink/DoCanDataLinkLayer.h"
#include "docan/datalink/DoCanFdFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"
#include "docan/datalink/DoCanFrameReceiverMock.h"

#include <etl/span.h>

#include <gmock/gmock.h>

namespace
{
using namespace docan;
using namespace testing;

using DataLinkLayerType       = DoCanDataLinkLayer<uint32_t, uint16_t, uint8_t, 0xFFFFFFFFU>;
using CodecType               = DoCanFrameCodec<DataLinkLayerType>;
using ConnectionType          = DoCanConnection<DataLinkLayerType>;
using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;

struct DoCanFrameDecoderTest : Test
{
    DoCanFrameDecoderTest() : _codec(DoCanFrameCodecConfigPresets::OPTIMIZED_CLASSIC, _sizeMapper)
    {}

    DoCanFdFrameSizeMapper<uint8_t> const _sizeMapper;
    CodecType _codec;
    StrictMock<DoCanFrameReceiverMock<DataLinkLayerType>> _frameReceiverMock;
};

TEST_F(DoCanFrameDecoderTest, testDecodeSingleFrame)
{
    DoCanFrameDecoder<DataLinkLayerType> cut;
    DoCanConnection<DataLinkLayerType> conn(
        _codec, DataLinkAddressPairType(0x1, 0x3), DoCanTransportAddressPair(0x3, 0x4));
    {
        // valid frame
        uint8_t const payload[] = {0x02, 0x13, 0x24};
        auto span               = ::etl::span<uint8_t const>(payload).subspan(1U);
        EXPECT_CALL(
            _frameReceiverMock,
            firstDataFrameReceived(conn, 2U, 1U, 0U, ElementsAreArray(span.data(), span.size())));
        EXPECT_EQ(CodecResult::OK, cut.decodeFrame(conn, payload, _frameReceiverMock));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
    {
        // invalid frame
        uint8_t const payload[] = {0x02, 0x24};
        EXPECT_EQ(
            CodecResult::INVALID_MESSAGE_SIZE, cut.decodeFrame(conn, payload, _frameReceiverMock));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
}

/**
 * \req: [BSW_DoCAN_73]
 */
TEST_F(DoCanFrameDecoderTest, testDecodeFirstFrame)
{
    DoCanFrameDecoder<DataLinkLayerType> cut;
    DoCanConnection<DataLinkLayerType> conn(
        _codec, DataLinkAddressPairType(0x1, 0x3), DoCanTransportAddressPair(0x3, 0x4));
    {
        // valid frame
        uint8_t const payload[] = {0x10, 0x12, 0x24, 0x45, 0x67, 0x89, 0x9a, 0x91};
        auto span               = ::etl::span<uint8_t const>(payload).subspan(2U);
        EXPECT_CALL(
            _frameReceiverMock,
            firstDataFrameReceived(
                conn, 0x12U, 3U, 7U, ElementsAreArray(span.data(), span.size())));
        EXPECT_EQ(CodecResult::OK, cut.decodeFrame(conn, payload, _frameReceiverMock));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
    {
        // valid frame with escape sequence
        uint8_t const payload[] = {0x10, 0x00, 0x00, 0x00, 0xA5, 0xB4, 0x5A, 0x4B};
        auto span               = ::etl::span<uint8_t const>(payload).subspan(6U);
        EXPECT_CALL(
            _frameReceiverMock,
            firstDataFrameReceived(
                conn, 0xA5B4U, 6061U, 7U, ElementsAreArray(span.data(), span.size())));
        EXPECT_EQ(CodecResult::OK, cut.decodeFrame(conn, payload, _frameReceiverMock));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
    {
        // invalid frame
        uint8_t const payload[] = {0x12, 0x24};
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_SIZE, cut.decodeFrame(conn, payload, _frameReceiverMock));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
}

TEST_F(DoCanFrameDecoderTest, testDecodeConsecutiveFrame)
{
    DoCanFrameDecoder<DataLinkLayerType> cut;
    DoCanConnection<DataLinkLayerType> conn(
        _codec, DataLinkAddressPairType(0x1, 0x3), DoCanTransportAddressPair(0x3, 0x4));
    {
        // valid frame
        uint8_t const payload[] = {0x21, 0x12, 0x24, 0x45, 0x67, 0x89, 0x9a, 0x91};
        auto span               = ::etl::span<uint8_t const>(payload).subspan(1U);
        EXPECT_CALL(
            _frameReceiverMock,
            consecutiveDataFrameReceived(0x01, 1U, ElementsAreArray(span.data(), span.size())));
        EXPECT_EQ(CodecResult::OK, cut.decodeFrame(conn, payload, _frameReceiverMock));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
    {
        // invalid frame
        uint8_t const payload[] = {0x21};
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_SIZE, cut.decodeFrame(conn, payload, _frameReceiverMock));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
}

TEST_F(DoCanFrameDecoderTest, testDecodeFlowControlFrame)
{
    DoCanFrameDecoder<DataLinkLayerType> cut;
    DoCanConnection<DataLinkLayerType> conn(
        _codec, DataLinkAddressPairType(0x1, 0x3), DoCanTransportAddressPair(0x3, 0x4));
    {
        // valid frame
        uint8_t const payload[] = {0x30, 0x12, 0x24};
        EXPECT_CALL(
            _frameReceiverMock, flowControlFrameReceived(0x1U, FlowStatus::CTS, 0x12, 0x24));
        EXPECT_EQ(CodecResult::OK, cut.decodeFrame(conn, payload, _frameReceiverMock));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
    {
        // invalid frame
        uint8_t const payload[] = {0x34, 0x12};
        EXPECT_EQ(
            CodecResult::INVALID_FRAME_SIZE, cut.decodeFrame(conn, payload, _frameReceiverMock));
        Mock::VerifyAndClearExpectations(&_frameReceiverMock);
    }
}

TEST_F(DoCanFrameDecoderTest, testDecodeFrameWithInvalidFrame)
{
    DoCanFrameDecoder<DataLinkLayerType> cut;
    DoCanConnection<DataLinkLayerType> conn(
        _codec, DataLinkAddressPairType(0x1, 0x3), DoCanTransportAddressPair(0x3, 0x4));
    EXPECT_EQ(CodecResult::INVALID_FRAME_SIZE, cut.decodeFrame(conn, {}, _frameReceiverMock));
}

TEST_F(DoCanFrameDecoderTest, testDecodeFrameWithUnknownFrameType)
{
    DoCanFrameDecoder<DataLinkLayerType> cut;
    DoCanConnection<DataLinkLayerType> conn(
        _codec, DataLinkAddressPairType(0x1, 0x3), DoCanTransportAddressPair(0x3, 0x4));
    uint8_t const payload[] = {0x50, 0x12U};
    EXPECT_EQ(CodecResult::INVALID_FRAME_TYPE, cut.decodeFrame(conn, payload, _frameReceiverMock));
}

} // anonymous namespace
