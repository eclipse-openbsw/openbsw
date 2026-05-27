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

#include "ip/NetworkInterfaceConfig.h"

#include <gmock/gmock.h>

namespace ip
{
struct NetworkInterfaceConfigRegistryMock : public ::ip::declare::NetworkInterfaceConfigRegistry<2>
{
    NetworkInterfaceConfigRegistryMock() : ::ip::declare::NetworkInterfaceConfigRegistry<2>({}, {})
    {}

    MOCK_METHOD(::ip::NetworkInterfaceConfig, getConfig, (uint8_t), (const, override));
};

} // namespace ip
