// Copyright 2025 Accenture.

#pragma once

#include "async/Async.h"
#include "async/IRunnable.h"
#include "async/util/Call.h"
#include "console/AsyncCommandWrapper.h"
#include "ethernet/EnetDriver.h"
#include "lifecycle/AsyncLifecycleComponent.h"
#include "lifecycle/ILifecycleManager.h"
#include "phy/Tja1101.h"
#include "phy/Tja1101Tester.h"
#include "systems/IEthernetDriverSystem.h"

namespace systems
{

class S32K148EvbEthernetSystem final
: public ::ethernet::IEthernetDriverSystem
, private ::async::IRunnable
, public ::lifecycle::AsyncLifecycleComponent
{
public:
    explicit S32K148EvbEthernetSystem(
        ::async::ContextType context, ::lifecycle::ILifecycleManager& lifecycleManager);

    void init() override;
    void run() override;
    void shutdown() override;
    void execute() override;

    void setGroupcastAddressRecognition(uint8_t const* const mac) const override
    {
        _driver.setGroupcastAddressRecognition(mac);
    }

    bool getLinkStatus(size_t port) override;

    bool writeFrame(struct netif* const aNetif, struct pbuf* const buf) override
    {
        return _driver.writeFrame(aNetif, buf);
    }

    ::lwiputils::PbufQueue::Receiver getRx() override
    {
        return ::lwiputils::PbufQueue::Receiver(_driver._rxBuffers._queue);
    }

    ::async::ContextType const _context;
    ::async::TimeoutType _timeout;
    ::async::Function _asyncReceiveTask;
    ::ethernet::EnetDriver::Configuration _driverConfig;
    ::ethernet::EnetDriver _driver;

    ::enetphy::MdioTja1101 _mdioTja1101;
    ::enetphy::Tja1101 _tja1101;
    ::enetphy::Tja1101Tester _tja1101Tester;
    ::console::AsyncCommandWrapper _asyncTja1101Tester;
    bool _tja1101LinkStatus;
};

} // namespace systems
