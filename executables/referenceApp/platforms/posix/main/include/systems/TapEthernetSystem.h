/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "TapEthernetDriver.h"

#include <ethernet/EthernetLogger.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <systems/IEthernetDriverSystem.h>

namespace systems
{

class TapEthernetSystem final
: public ::ethernet::IEthernetDriverSystem
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

    ::lwiputils::PbufQueue& getRx() override { return _driver._queue; }

    ::async::ContextType _context;
    ::async::TimeoutType _rxTimeout;
    ::ethernet::TapEthernetDriver _driver;
};

} // namespace systems
