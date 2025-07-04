// Copyright 2025 Accenture.

#pragma once

#include <async/Async.h>
#include <async/IRunnable.h>
#include <console/AsyncCommandWrapper.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <lifecycle/ILifecycleListener.h>
#include <lifecycle/console/LifecycleControlCommand.h>

namespace systems
{

class BackgroundSystem
: public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
, private ::lifecycle::ILifecycleListener
{
public:
    explicit BackgroundSystem(
        ::async::ContextType context, ::lifecycle::ILifecycleManager& lifecycleManager);

    BackgroundSystem(BackgroundSystem const&)            = delete;
    BackgroundSystem& operator=(BackgroundSystem const&) = delete;

    void init() override;
    void run() override;
    void shutdown() override;

    void cyclic();

private:
    void execute() override;

    void lifecycleLevelReached(
        uint8_t const level,
        ::lifecycle::ILifecycleComponent::Transition::Type const /* transition */) override;

private:
    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
};

} // namespace systems
