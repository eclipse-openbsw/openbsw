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
