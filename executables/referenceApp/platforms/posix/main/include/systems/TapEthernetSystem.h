// Copyright 2025 Accenture.

#pragma once

#include "TapEthernetDriver.h"

#include <ethernet/EthernetLogger.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <systems/IEthernetSystem.h>

namespace systems
{

class TapEthernetSystem final
: public ::ethernet::IEthernetSystem
, public ::lifecycle::AsyncLifecycleComponent
, public ::async::IRunnable
{
public:
    explicit TapEthernetSystem(::async::ContextType context);

    void init() override;

    void run() override;

    void shutdown() override;

    void execute() override;

    void setGroupcastAddressRecognition(::etl::array<uint8_t, 6> const /* mac */) const override {}

    bool getLinkStatus(size_t port) override;

    bool writeFrame(netif* ni, pbuf* pb) override;

    ::lwiputils::PbufQueue::Receiver getRx() override
    {
        return ::lwiputils::PbufQueue::Receiver(_driver._queue);
    }

    ::async::ContextType _context;
    ::async::TimeoutType _rxTimeout;
    ::ethernet::TapEthernetDriver _driver;
};

} // namespace systems
