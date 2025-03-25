// Copyright 2024 Accenture.

#pragma once

#include "docan/common/DoCanConstants.h"
#include "docan/datalink/IDoCanDataFrameTransmitter.h"

#include <gmock/gmock.h>

namespace docan
{
/**
 * Interface for DoCan frame receiver.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanFrameReceiverMock : public IDoCanFrameReceiver<DataLinkLayer>
{
public:
    using DataLinkAddressType = typename DataLinkLayer::AddressType;
    using MessageSizeType     = typename DataLinkLayer::MessageSizeType;
    using FrameSizeType       = typename DataLinkLayer::FrameSizeType;
    using FrameIndexType      = typename DataLinkLayer::FrameIndexType;
    using ConnectionType      = DoCanConnection<DataLinkLayer>;

    MOCK_METHOD5_T(
        firstDataFrameReceived,
        void(
            ConnectionType const& connection,
            MessageSizeType messageSize,
            FrameIndexType frameCount,
            FrameSizeType consecutiveFrameDataSize,
            ::estd::slice<uint8_t const> const& data));
    MOCK_METHOD3_T(
        consecutiveDataFrameReceived,
        void(
            DataLinkAddressType receptionAddress,
            uint8_t sequenceNumber,
            ::estd::slice<uint8_t const> const& data));
    MOCK_METHOD4_T(
        flowControlFrameReceived,
        void(
            DataLinkAddressType receptionAddress,
            FlowStatus flowStatus,
            uint8_t blockSize,
            uint8_t encodedMinSeparationTime));
};

} // namespace docan

