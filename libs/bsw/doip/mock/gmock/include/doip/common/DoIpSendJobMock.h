/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * \ingroup doip
 */
#pragma once

#include "doip/common/IDoIpSendJob.h"

#include <gmock/gmock.h>

namespace doip
{
/**
 * Mock for a DoIp send job.
 */
class DoIpSendJobMock : public IDoIpSendJob
{
public:
    MOCK_CONST_METHOD0(getSendBufferCount, uint8_t());
    MOCK_CONST_METHOD0(getTotalLength, uint16_t());
    MOCK_CONST_METHOD0(getDestinationEndpoint, ::ip::IPEndpoint const*());
    MOCK_METHOD1(release, void(bool success));
    MOCK_METHOD2(
        getSendBuffer,
        ::etl::span<uint8_t const>(::etl::span<uint8_t> staticBuffer, uint8_t index));
};

} // namespace doip
