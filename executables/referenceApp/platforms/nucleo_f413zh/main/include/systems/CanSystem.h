// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file CanSystem.h
 * \brief CAN system lifecycle component for the NUCLEO-F413ZH platform (bxCAN).
 *
 * Provides the platform-specific CanSystem that manages a single BxCanTransceiver
 * instance on CAN1 and integrates with the openBSW lifecycle manager.
 */

#pragma once

#include <async/Async.h>
#include <can/transceiver/bxcan/BxCanTransceiver.h>
#include <etl/singleton_base.h>
#include <lifecycle/SingleContextLifecycleComponent.h>
#include <systems/ICanSystem.h>

namespace systems
{

/**
 * Platform-specific CAN system for the STM32F413ZH NUCLEO board.
 *
 * Manages a single BxCanTransceiver on CAN1 (PD0 RX / PD1 TX, 500 kbit/s).
 * Implements the ICanSystem interface and participates in the openBSW lifecycle
 * as a SingleContextLifecycleComponent. Registered as an etl::singleton so the
 * ISR trampolines can reach the instance without global variables.
 */
class CanSystem
: public ::lifecycle::SingleContextLifecycleComponent
, public ::can::ICanSystem
, public ::etl::singleton_base<CanSystem>
{
public:
    /**
     * Constructs the CanSystem.
     *
     * \param context The async context (task) in which CAN processing will run.
     */
    CanSystem(::async::ContextType context);

    /**
     * Initializes the CanSystem.
     * Invokes transitionDone to notify the lifecycle manager that the component
     * has completed its init transition.
     */
    void init() override;

    /**
     * Runs the CanSystem.
     * Enables CanRxRunnable, initializes and opens the BxCanTransceiver to set up
     * the CAN1 peripheral, configures NVIC priorities and enables CAN IRQs,
     * then invokes transitionDone.
     */
    void run() override;

    /**
     * Shuts down the CanSystem.
     * Closes and shuts down the BxCanTransceiver, disables CanRxRunnable, then
     * invokes transitionDone.
     */
    void shutdown() override;

    /**
     * Returns the CAN transceiver for the given bus ID.
     *
     * \param busId The bus identifier to look up.
     * \return Pointer to the BxCanTransceiver if busId matches CAN_0, nullptr otherwise.
     */
    ::can::ICanTransceiver* getCanTransceiver(uint8_t busId) override;

    /**
     * Dispatches the CAN RX processing runnable into the async context.
     *
     * \note Called from ISR context (call_can_isr_RX). Must be safe for ISR invocation.
     */
    void dispatchRxTask();

private:
    /**
     * Async runnable that processes received CAN frames outside ISR context.
     */
    class CanRxRunnable : public ::async::RunnableType
    {
    public:
        /**
         * Constructs the CanRxRunnable.
         *
         * \param parent The owning CanSystem instance.
         */
        explicit CanRxRunnable(CanSystem& parent);

        /**
         * Executes CAN frame reception processing.
         * Called by the async framework when dispatched from the RX ISR.
         */
        void execute() override;

        /**
         * Enables or disables the runnable.
         *
         * \param enabled true to allow execute() to process frames, false to skip.
         */
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
