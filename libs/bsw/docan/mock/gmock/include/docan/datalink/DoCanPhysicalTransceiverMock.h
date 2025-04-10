// Copyright 2024 Accenture.

#pragma once

#include "docan/common/DoCanConstants.h"
#include "docan/datalink/IDoCanPhysicalTransceiver.h"

#include <gmock/gmock.h>

namespace docan
{
/**
 * Interface for an abstract frame transceiver.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanPhysicalTransceiverMock : public IDoCanPhysicalTransceiver<DataLinkLayer>
{
public:
    using DataLinkLayerType   = DataLinkLayer;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;
    using MessageSizeType     = typename DataLinkLayerType::MessageSizeType;
    using FrameSizeType       = typename DataLinkLayerType::FrameSizeType;
    using FrameIndexType      = typename DataLinkLayerType::FrameIndexType;
    using JobHandleType       = typename DataLinkLayerType::JobHandleType;
    using FrameCodecType      = DoCanFrameCodec<DataLinkLayerType>;

    MOCK_METHOD1_T(init, void(IDoCanFrameReceiver<DataLinkLayer>& receiver));
    MOCK_METHOD0(shutdown, void());

    MOCK_METHOD8_T(
        startSendDataFrames,
        SendResult(
            FrameCodecType const& codec,
            IDoCanDataFrameTransmitterCallback<DataLinkLayer>& callback,
            JobHandleType jobHandle,
            DataLinkAddressType transmissionAddress,
            FrameIndexType firstFrameIndex,
            FrameIndexType lastFrameIndex,
            FrameSizeType consecutiveFrameDataSize,
            ::etl::span<uint8_t const> const& data));
    MOCK_METHOD2_T(
        cancelSendDataFrames,
        void(IDoCanDataFrameTransmitterCallback<DataLinkLayer>& callback, JobHandleType jobHandle));

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
