// Copyright 2025 Accenture.

#pragma once

#include "ip/INetworkInterfaceConfigRegistry.h"

#include <gmock/gmock.h>

namespace ip
{
struct NetworkInterfaceConfigRegistryMock
{
    MOCK_METHOD0(configChangedSignal, ConfigChangedSignal&());
    MOCK_METHOD1(updateConfig, void(NetworkInterfaceConfig const&));
};

} // namespace ip
