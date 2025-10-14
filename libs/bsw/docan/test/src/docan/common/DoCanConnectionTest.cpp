// Copyright 2024 Accenture.

#include "docan/common/DoCanConnection.h"

#include "docan/common/DoCanTransportAddressPair.h"
#include "docan/datalink/DoCanDataLinkLayer.h"
#include "docan/datalink/DoCanFdFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodec.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

#include <gmock/gmock.h>

namespace
{
using namespace docan;

using DataLinkLayerType       = DoCanDataLinkLayer<uint32_t, uint16_t, uint8_t, 0xFFFFFFFFU>;
using CodecType               = DoCanFrameCodec<DataLinkLayerType>;
using DataLinkAddressPairType = typename DataLinkLayerType::AddressPairType;

/*
 * Ensure that operator== works as expected
 */
TEST(DoCanConnectionTest, connectionEquality)
{
    DoCanFdFrameSizeMapper<uint8_t> const _sizeMapper;
    CodecType _paddedClassicCodec(DoCanFrameCodecConfigPresets::PADDED_CLASSIC, _sizeMapper);
    CodecType _paddedFdCodec(DoCanFrameCodecConfigPresets::PADDED_FD, _sizeMapper);

    DoCanConnection<DataLinkLayerType> conn(
        _paddedClassicCodec,
        DataLinkAddressPairType(0x1, 0x3),
        DoCanTransportAddressPair(0x3, 0x4));
    DoCanConnection<DataLinkLayerType> connEq(
        _paddedClassicCodec,
        DataLinkAddressPairType(0x1, 0x3),
        DoCanTransportAddressPair(0x3, 0x4));

    DoCanConnection<DataLinkLayerType> connDifferentCodec(
        _paddedFdCodec, DataLinkAddressPairType(0x1, 0x3), DoCanTransportAddressPair(0x3, 0x4));

    DoCanConnection<DataLinkLayerType> connDifferentDataLinkAddress(
        _paddedClassicCodec,
        DataLinkAddressPairType(0x1, 0x4),
        DoCanTransportAddressPair(0x3, 0x4));

    DoCanConnection<DataLinkLayerType> connDifferentTransportAddress(
        _paddedClassicCodec,
        DataLinkAddressPairType(0x1, 0x3),
        DoCanTransportAddressPair(0x3, 0x5));

    EXPECT_EQ(conn, connEq);

    // Uses EXPECT_FALSE so we don't have to needlessly implement operator!= just for this test case
    EXPECT_FALSE(conn == connDifferentCodec);
    EXPECT_FALSE(conn == connDifferentDataLinkAddress);
    EXPECT_FALSE(conn == connDifferentTransportAddress);
}

} // namespace
