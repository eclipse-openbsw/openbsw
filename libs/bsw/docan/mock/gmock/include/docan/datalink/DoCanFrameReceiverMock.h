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

    MOCK_METHOD(
        void,
        firstDataFrameReceived,
        (ConnectionType const& connection,
         MessageSizeType messageSize,
         FrameIndexType frameCount,
         FrameSizeType consecutiveFrameDataSize,
         ::etl::span<uint8_t const> const& data));
    MOCK_METHOD(
        void,
        consecutiveDataFrameReceived,
        (DataLinkAddressType receptionAddress,
         uint8_t sequenceNumber,
         ::etl::span<uint8_t const> const& data));
    MOCK_METHOD(
        void,
        flowControlFrameReceived,
        (DataLinkAddressType receptionAddress,
         FlowStatus flowStatus,
         uint8_t blockSize,
         uint8_t encodedMinSeparationTime));
};

} // namespace docan
