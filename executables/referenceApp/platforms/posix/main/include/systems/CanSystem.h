// Copyright 2024 Accenture.

#pragma once

#include <can/SocketCanTransceiver.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <systems/ICanSystem.h>

namespace systems
{

class CanSystem final
: public ::can::ICanSystem
, public ::lifecycle::AsyncLifecycleComponent
, private ::async::IRunnable
{
public:
    // [PUBLIC_API_START]
    explicit CanSystem(::async::ContextType context);
    CanSystem(CanSystem const&)            = delete;
    CanSystem& operator=(CanSystem const&) = delete;

    ::can::ICanTransceiver* getCanTransceiver(uint8_t busId) override;

    void init() final;
    void run() final;
    void shutdown() final;
    // [PUBLIC_API_END]
private:
    void execute() final;

private:
    ::async::TimeoutType _timeout;
    ::async::ContextType _context;

    ::can::SocketCanTransceiver _canTransceiver;
};

} // namespace systems
