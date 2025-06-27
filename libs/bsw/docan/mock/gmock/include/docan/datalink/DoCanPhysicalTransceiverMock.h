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

    MOCK_METHOD(void, init, (IDoCanFrameReceiver<DataLinkLayer> & receiver), (override));
    MOCK_METHOD(void, shutdown, (), (override));

    MOCK_METHOD(
        SendResult,
        startSendDataFrames,
        (FrameCodecType const& codec,
         IDoCanDataFrameTransmitterCallback<DataLinkLayer>& callback,
         JobHandleType jobHandle,
         DataLinkAddressType transmissionAddress,
         FrameIndexType firstFrameIndex,
         FrameIndexType lastFrameIndex,
         FrameSizeType consecutiveFrameDataSize,
         ::estd::slice<uint8_t const> const& data),
        (override));
    MOCK_METHOD(
        void,
        cancelSendDataFrames,
        (IDoCanDataFrameTransmitterCallback<DataLinkLayer> & callback, JobHandleType jobHandle),
        (override));

    MOCK_METHOD(
        bool,
        sendFlowControl,
        (FrameCodecType const& codec,
         DataLinkAddressType transmissionAddress,
         FlowStatus flowStatus,
         uint8_t blockSize,
         uint8_t encodedMinSeparationTime),
        (override));
};

} // namespace docan
