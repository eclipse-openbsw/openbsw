/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
         ::etl::span<uint8_t const> const& data));
    MOCK_METHOD(
        void,
        cancelSendDataFrames,
        (IDoCanDataFrameTransmitterCallback<DataLinkLayer> & callback, JobHandleType jobHandle));
};

} // namespace docan
