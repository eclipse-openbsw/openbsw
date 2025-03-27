// Copyright 2024 Accenture.

#pragma once

#include <async/Async.h>
#include <async/IRunnable.h>
#include <console/AsyncCommandWrapper.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/console/LifecycleControlCommand.h>

namespace systems
{

class SysAdminSystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    explicit SysAdminSystem(
        ::async::ContextType context, ::lifecycle::ILifecycleManager& lifecycleManager);
    SysAdminSystem(SysAdminSystem const&)            = delete;
    SysAdminSystem& operator=(SysAdminSystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;

private:
    void execute() override;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;

    ::lifecycle::LifecycleControlCommand _lifecycleControlCommand;
    ::console::AsyncCommandWrapper _asyncCommandWrapperForLifecycleControlCommand;
};

} // namespace systems
