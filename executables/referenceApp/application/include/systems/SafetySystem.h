/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/ILifecycleManager.h>

#if PLATFORM_SUPPORT_IO
#include <console/AsyncCommandWrapper.h>
#include <safety/console/SafetyCommand.h>
#endif

namespace systems
{
class SafetySystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    explicit SafetySystem(
        ::async::ContextType context, ::lifecycle::ILifecycleManager& lifecycleManager);
    SafetySystem(SafetySystem const&)            = delete;
    SafetySystem& operator=(SafetySystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;
    void cyclic();

private:
    void execute() override;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
#if PLATFORM_SUPPORT_IO
    ::safety::SafetyCommand _safetyCommand;
    ::console::AsyncCommandWrapper _asyncCommandWrapperForSafetyCommand;
#endif
};

} // namespace systems
