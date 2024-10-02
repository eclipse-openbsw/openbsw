// Copyright 2024 Accenture.

#ifndef GUARD_DC58DFB6_9164_4A62_A31A_6DDB9AFBC5D4
#define GUARD_DC58DFB6_9164_4A62_A31A_6DDB9AFBC5D4

#include "docan/common/DoCanConstants.h"
#include "docan/datalink/IDoCanFlowControlFrameTransmitter.h"

#include <gmock/gmock.h>

namespace docan
{
/**
 * Callback interface for DoCan flow control frame transmitter.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanFlowControlFrameTransmitterMock : public IDoCanFlowControlFrameTransmitter<DataLinkLayer>
{
public:
    using DataLinkAddressType = typename DataLinkLayer::AddressType;
    using FrameCodecType      = DoCanFrameCodec<DataLinkLayer>;

    MOCK_METHOD5_T(
        sendFlowControl,
        bool(
            FrameCodecType const& codec,
            DataLinkAddressType transmissionAddress,
            FlowStatus flowStatus,
            uint8_t blockSize,
            uint8_t encodedMinSeparationTime));
};

} // namespace docan

#endif // GUARD_DC58DFB6_9164_4A62_A31A_6DDB9AFBC5D4
