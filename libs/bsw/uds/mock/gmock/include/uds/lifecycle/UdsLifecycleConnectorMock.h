// Copyright 2024 Accenture.

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
