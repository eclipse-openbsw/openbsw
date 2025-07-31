// Copyright 2025 Accenture.

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
