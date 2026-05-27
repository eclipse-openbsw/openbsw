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

#include "doip/server/IDoIpServerConnectionFilter.h"

#include <etl/optional.h>

#include <gmock/gmock.h>

namespace doip
{
/**
 * Filter interface for a DoIP server connection.
 */
class DoIpServerConnectionFilterMock : public IDoIpServerConnectionFilter
{
public:
    MOCK_METHOD7(
        checkRoutingActivation,
        RoutingActivationCheckResult(
            uint16_t sourceAddress,
            uint8_t activationType,
            uint8_t socketGroupId,
            ::ip::IPEndpoint const& localEndpoint,
            ::ip::IPEndpoint const& remoteEndpoint,
            ::etl::optional<uint32_t> const oemField,
            bool isResuming));
};

} // namespace doip
