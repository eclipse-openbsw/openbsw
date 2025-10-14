// Copyright 2024 Accenture.

#include "docan/can/DoCanPhysicalCanTransceiverContainer.h"

#include "docan/addressing/DoCanAddressConverterMock.h"
#include "docan/addressing/DoCanNormalAddressing.h"
#include "docan/datalink/DoCanFdFrameSizeMapper.h"
#include "docan/datalink/DoCanFrameCodecConfigPresets.h"

#include <can/filter/FilterMock.h>
#include <can/filter/IMerger.h>
#include <can/transceiver/ICanTransceiverMock.h>

#include <gtest/gtest.h>

namespace
{
using namespace can;
using namespace docan;
using namespace testing;

using AddressingType = DoCanNormalAddressing<>;

struct DoCanPhysicalCanTransceiverContainerTest : ::testing::Test
{
    DoCanPhysicalCanTransceiverContainerTest() {}

    DoCanFdFrameSizeMapper<uint8_t> const _sizeMapper;
    AddressingType _addressing;
    StrictMock<DoCanAddressConverterMock<AddressingType::DataLinkLayerType>> _addressConverterMock;
    StrictMock<FilterMock> _filterMock;
    StrictMock<ICanTransceiverMock> _canTransceiverMock1;
    StrictMock<ICanTransceiverMock> _canTransceiverMock2;
};

TEST_F(DoCanPhysicalCanTransceiverContainerTest, testConstructedCanTransceivers)
{
    ::docan::declare::DoCanPhysicalCanTransceiverContainer<AddressingType, 2> cut;
    ::docan::declare::DoCanPhysicalCanTransceiverContainerBuilder<AddressingType> builder(
        cut, _filterMock, _addressConverterMock, _addressing);
    EXPECT_EQ(0U, cut.getTransceivers().size());
    DoCanPhysicalCanTransceiver<AddressingType>& transceiver1
        = builder.addTransceiver(_canTransceiverMock1);
    EXPECT_EQ(1U, cut.getTransceivers().size());
    EXPECT_EQ(&transceiver1, &cut.getTransceivers()[0]);
    DoCanPhysicalCanTransceiver<AddressingType>& transceiver2
        = builder.addTransceiver(_canTransceiverMock2);
    EXPECT_EQ(2U, cut.getTransceivers().size());
    EXPECT_EQ(&transceiver1, &cut.getTransceivers()[0]);
    EXPECT_EQ(&transceiver2, &cut.getTransceivers()[1]);
}

} // anonymous namespace
