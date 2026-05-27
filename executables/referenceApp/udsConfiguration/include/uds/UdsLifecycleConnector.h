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
