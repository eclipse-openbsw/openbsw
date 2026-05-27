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

#include "uds/lifecycle/IUdsLifecycleConnector.h"

#include <gmock/gmock.h>

namespace uds
{
class UdsLifecycleConnectorMock : public IUdsLifecycleConnector
{
public:
    MOCK_METHOD(bool, isModeChangePossible, (), (const, override));
    MOCK_METHOD(bool, requestPowerdown, (bool rapid, uint8_t& time), (override));
    MOCK_METHOD(bool, requestShutdown, (ShutdownType type, uint32_t timeout), (override));
};

} // namespace uds
