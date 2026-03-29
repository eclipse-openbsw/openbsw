// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file BxCanTransceiver.h
 * \brief OpenBSW CAN transceiver for the STM32 bxCAN peripheral.
 * \ingroup transceiver
 *
 * Matches the S32K CanFlex2Transceiver interface contract:
 * - TX with listener: async callback via TX ISR → async::execute → task context
 * - TX without listener: fire-and-forget with synchronous notifySentListeners
 * - RX: ISR accepts all frames, per-listener filtering in notifyListeners
 */
#pragma once

#include <async/Async.h>
#include <async/util/Call.h>
#include <can/BxCanDevice.h>
#include <can/canframes/ICANFrameSentListener.h>
#include <can/transceiver/AbstractCANTransceiver.h>

#include <etl/deque.h>

namespace bios
{

/**
 * CAN transceiver for STM32 bxCAN peripheral.
 *
 * Wraps BxCanDevice to implement the AbstractCANTransceiver interface.
 * Operates in classic CAN mode at 500 kbps on STM32F4 family devices.
 *
 * \see AbstractCANTransceiver
 * \see BxCanDevice
 */
class BxCanTransceiver : public ::can::AbstractCANTransceiver
{
public:
    BxCanTransceiver(
        ::async::ContextType context, uint8_t busId, BxCanDevice::Config const& devConfig);

    ErrorCode init() override;
    ErrorCode open() override;
    ErrorCode open(::can::CANFrame const& frame) override;
    ErrorCode close() override;
    void shutdown() override;
    ErrorCode mute() override;
    ErrorCode unmute() override;

    /// Fire-and-forget transmit. Calls notifySentListeners() synchronously.
    ErrorCode write(::can::CANFrame const& frame) override;

    /// Transmit with deferred callback. Queues {listener, frame} into fTxQueue.
    /// canFrameSent() fires from task context after TX ISR, NOT from write().
    ErrorCode write(::can::CANFrame const& frame, ::can::ICANFrameSentListener& listener) override;

    uint32_t getBaudrate() const override;
    uint16_t getHwQueueTimeout() const override;

    /// Number of TX frames dropped due to HW queue full or TX listener queue full.
    uint32_t getOverrunCount() const { return fOverrunCount; }

    static uint8_t receiveInterrupt(uint8_t transceiverIndex);
    static void transmitInterrupt(uint8_t transceiverIndex);
    static void disableRxInterrupt(uint8_t transceiverIndex);
    static void enableRxInterrupt(uint8_t transceiverIndex);

    void cyclicTask();
    void receiveTask();

    /// Underlying bxCAN hardware device. Public for test access.
    BxCanDevice fDevice;

private:
    /// TX job queued for async callback — matches S32K TxJobWithCallback.
    struct TxJobWithCallback
    {
        TxJobWithCallback(::can::ICANFrameSentListener& listener, ::can::CANFrame const& frame)
        : _listener(listener), _frame(frame)
        {}

        ::can::ICANFrameSentListener& _listener;
        ::can::CANFrame _frame;
    };

    static uint32_t const TX_QUEUE_CAPACITY = 3U;
    using TxQueue                           = ::etl::deque<TxJobWithCallback, TX_QUEUE_CAPACITY>;

    /// Called from TX ISR — defers to task context via async::execute.
    void canFrameSentCallback();

    /// Runs in task context — pops TX queue, calls listener, sends next frame.
    void canFrameSentAsyncCallback();

    /// Wraps notifySentListeners for consistency with S32K naming.
    void notifyRegisteredSentListener(::can::CANFrame const& frame) { notifySentListeners(frame); }

    static BxCanTransceiver* fpTransceivers[3];

    ::async::ContextType fContext;
    ::async::TimeoutType fCyclicTimeout;
    ::async::Function _cyclicTaskRunner;
    ::async::Function _canFrameSent;
    TxQueue fTxQueue;
    uint32_t fOverrunCount;
    bool fMuted;
};

} // namespace bios
