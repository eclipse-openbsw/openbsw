/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <async/Async.h>
#include <can/transceiver/bxcan/BxCanTransceiver.h>
#include <etl/singleton_base.h>
#include <lifecycle/SingleContextLifecycleComponent.h>
#include <systems/ICanSystem.h>

namespace systems
{

/**
 * CAN system for the NUCLEO-F413ZH: manages a single BxCanTransceiver on CAN1
 * (PD0 RX / PD1 TX, 500 kbit/s). Registered as an etl::singleton so the ISR
 * trampolines can reach the instance without global variables.
 */
class CanSystem
: public ::lifecycle::SingleContextLifecycleComponent
, public ::can::ICanSystem
, public ::etl::singleton_base<CanSystem>
{
public:
    CanSystem(::async::ContextType context);

    void init() override;
    void run() override;
    void shutdown() override;

    ::can::ICanTransceiver* getCanTransceiver(uint8_t busId) override;

    /**
     * Dispatches CAN RX processing into the async context.
     * \note Called from ISR context (call_can_isr_RX). Must be safe for ISR invocation.
     */
    void dispatchRxTask();

private:
    class CanRxRunnable : public ::async::RunnableType
    {
    public:
        explicit CanRxRunnable(CanSystem& parent);

        void execute() override;

        void setEnabled(bool enabled) { _enabled = enabled; }

    private:
        CanSystem& _parent;
        bool _enabled;
    };

    ::async::ContextType _context;
    ::bios::BxCanTransceiver _transceiver0;
    CanRxRunnable _canRxRunnable;
};

} // namespace systems
