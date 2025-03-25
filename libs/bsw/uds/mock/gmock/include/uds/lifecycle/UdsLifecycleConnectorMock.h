// Copyright 2024 Accenture.

#pragma once

#include "uds/lifecycle/IUdsLifecycleConnector.h"

#include <gmock/gmock.h>

namespace uds
{
class UdsLifecycleConnectorMock : public IUdsLifecycleConnector
{
public:
    MOCK_CONST_METHOD0(isModeChangePossible, bool());
    MOCK_METHOD2(requestPowerdown, bool(bool rapid, uint8_t& time));
    MOCK_METHOD2(requestShutdown, bool(ShutdownType type, uint32_t timeout));
};

} // namespace uds

