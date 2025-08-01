// Copyright 2024 Accenture.

#pragma once

#include "docan/datalink/IDoCanDataFrameTransmitter.h"

#include <gmock/gmock.h>

namespace docan
{
/**
 * Interface for DoCan data frame transmitter.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanDataFrameTransmitterMock : public IDoCanDataFrameTransmitter<DataLinkLayer>
{
public:
    using DataLinkAddressType = typename DataLinkLayer::AddressType;
    using MessageSizeType     = typename DataLinkLayer::MessageSizeType;
    using FrameSizeType       = typename DataLinkLayer::FrameSizeType;
    using FrameIndexType      = typename DataLinkLayer::FrameIndexType;
    using JobHandleType       = typename DataLinkLayer::JobHandleType;
    using FrameCodecType      = DoCanFrameCodec<DataLinkLayer>;

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
};

} // namespace docan
