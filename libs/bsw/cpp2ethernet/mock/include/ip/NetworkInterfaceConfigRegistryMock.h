// Copyright 2025 Accenture.

#pragma once

#include "ip/INetworkInterfaceConfigRegistry.h"

#include <gmock/gmock.h>

namespace ip
{
struct NetworkInterfaceConfigRegistryMock
{
    MOCK_METHOD(ConfigChangedSignal&, configChangedSignal, ());
    MOCK_METHOD(void, updateConfig, (NetworkInterfaceConfig const&));
};

} // namespace ip
