/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

namespace ip
{
struct NetworkInterfaceConfigMock
{
    MOCK_CONST_METHOD0(isValid, bool());
    MOCK_CONST_METHOD0(ipFamily, ::ip::IPAddress::Family());
    MOCK_CONST_METHOD0(ipAddress, ::ip::IPAddress());
    MOCK_CONST_METHOD0(networkMask, ::ip::IPAddress());
    MOCK_CONST_METHOD0(defaultGateway, ::ip::IPAddress());
    MOCK_CONST_METHOD0(broadcastAddress, ::ip::IPAddress());
};

} // namespace ip
