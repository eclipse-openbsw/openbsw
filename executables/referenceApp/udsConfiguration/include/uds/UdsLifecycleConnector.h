// Copyright 2024 Accenture.

#pragma once

#include "uds/lifecycle/IUdsLifecycleConnector.h"

namespace lifecycle
{
class LifecycleManager;
}

namespace uds
{
class UdsLifecycleConnector : public IUdsLifecycleConnector
{
public:
    explicit UdsLifecycleConnector(lifecycle::LifecycleManager&) {}

    bool isModeChangePossible() const { return true; }

    bool requestPowerdown(bool /* rapid */, uint8_t& /* time */) { return true; }

    bool requestShutdown(ShutdownType /* type */, uint32_t) { return true; }
};

} // namespace uds
