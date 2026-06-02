/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <can/transceiver/bxcan/BxCanTransceiver.h>

#include <can/canframes/ICANFrameSentListener.h>

namespace bios
{

// Period of the cyclic bus-off polling task started by open()
static constexpr uint32_t BUS_OFF_POLL_PERIOD_MS = 10U;

BxCanTransceiver* BxCanTransceiver::fpTransceivers[NUMBER_OF_TRANSCEIVERS]
    = {nullptr, nullptr, nullptr};

BxCanTransceiver::BxCanTransceiver(
    ::async::ContextType context, uint8_t busId, BxCanDevice::Config const& devConfig)
: AbstractCANTransceiver(busId)
, fDevice(devConfig)
, fContext(context)
, fCyclicTimeout()
, _cyclicTaskRunner(
      ::async::Function::CallType::create<BxCanTransceiver, &BxCanTransceiver::cyclicTask>(*this))
, _canFrameSent(::async::Function::CallType::
                    create<BxCanTransceiver, &BxCanTransceiver::canFrameSentAsyncCallback>(*this))
, fTxQueue()
, fOverrunCount(0U)
, fMuted(false)
{
    if (busId < NUMBER_OF_TRANSCEIVERS)
    {
        fpTransceivers[busId] = this;
    }
}

::can::ICanTransceiver::ErrorCode BxCanTransceiver::init()
{
    if (getState() != State::CLOSED)
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }

    if (!fDevice.init())
    {
        return ErrorCode::CAN_ERR_INIT_FAILED;
    }
    setState(State::INITIALIZED);
    return ErrorCode::CAN_ERR_OK;
}

::can::ICanTransceiver::ErrorCode BxCanTransceiver::open()
{
    State const state = getState();
    if (state != State::INITIALIZED && state != State::CLOSED)
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }

    if (state == State::CLOSED)
    {
        if (!fDevice.init())
        {
            return ErrorCode::CAN_ERR_INIT_FAILED;
        }
    }

    if (!fDevice.start())
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::OPEN);
    fMuted = false;

    // Schedule periodic bus-off polling.
    ::async::scheduleAtFixedRate(
        fContext,
        _cyclicTaskRunner,
        fCyclicTimeout,
        BUS_OFF_POLL_PERIOD_MS,
        ::async::TimeUnit::MILLISECONDS);

    return ErrorCode::CAN_ERR_OK;
}

::can::ICanTransceiver::ErrorCode BxCanTransceiver::open(::can::CANFrame const& /* frame */)
{
    // Wake-up frame not supported on bxCAN
    return open();
}

::can::ICanTransceiver::ErrorCode BxCanTransceiver::close()
{
    if (getState() == State::CLOSED)
    {
        return ErrorCode::CAN_ERR_OK;
    }

    fCyclicTimeout.cancel();
    fDevice.stop();
    // Drop contract: queued listener jobs are discarded without a
    // canFrameSent notification - after close() the transceiver is offline
    // and consumers (e.g. DoCAN) recover via their own send timeout.
    fTxQueue.clear();
    setState(State::CLOSED);
    return ErrorCode::CAN_ERR_OK;
}

void BxCanTransceiver::shutdown() { close(); }

::can::ICanTransceiver::ErrorCode BxCanTransceiver::mute()
{
    if (getState() != State::OPEN)
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }

    fMuted = true;
    // Same drop contract as close(): pending listener jobs are discarded
    // without notification.
    fTxQueue.clear();
    setState(State::MUTED);
    return ErrorCode::CAN_ERR_OK;
}

::can::ICanTransceiver::ErrorCode BxCanTransceiver::unmute()
{
    if (getState() != State::MUTED)
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }

    fMuted = false;
    setState(State::OPEN);
    return ErrorCode::CAN_ERR_OK;
}

::can::ICanTransceiver::ErrorCode BxCanTransceiver::write(::can::CANFrame const& frame)
{
    if (getState() != State::OPEN || fMuted)
    {
        return ErrorCode::CAN_ERR_TX_OFFLINE;
    }

    if (!fDevice.transmit(frame))
    {
        fOverrunCount++;
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }

    notifySentListeners(frame);
    return ErrorCode::CAN_ERR_OK;
}

::can::ICanTransceiver::ErrorCode
BxCanTransceiver::write(::can::CANFrame const& frame, ::can::ICANFrameSentListener& listener)
{
    if (getState() != State::OPEN || fMuted)
    {
        return ErrorCode::CAN_ERR_TX_OFFLINE;
    }

    if (fTxQueue.full())
    {
        fOverrunCount++;
        notifyRegisteredSentListener(frame);
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }

    bool const wasEmpty = fTxQueue.empty();
    fTxQueue.emplace_back(listener, frame);

    if (!wasEmpty)
    {
        // Not the first in queue - will be sent from the TX ISR chain
        return ErrorCode::CAN_ERR_OK;
    }

    // First in queue - transmit now
    if (!fDevice.transmit(frame))
    {
        fTxQueue.pop_front();
        fOverrunCount++;
        notifyRegisteredSentListener(frame);
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }

    // Wait for TX ISR -> canFrameSentCallback() -> canFrameSentAsyncCallback()
    return ErrorCode::CAN_ERR_OK;
}

uint32_t BxCanTransceiver::getBaudrate() const { return fDevice.getBaudrate(); }

uint16_t BxCanTransceiver::getHwQueueTimeout() const
{
    // Time for all TX mailboxes to drain: TX_HW_FIFO_DEPTH frames x (53 bit
    // CAN overhead + 64 bit payload) at the configured bit rate, rounded up
    // to the next millisecond so the timeout is never 0 ms.
    uint32_t const baudrate = fDevice.getBaudrate();
    // NOLINTNEXTLINE(clang-analyzer-core.DivideZero): CAN baudrate can never be zero here.
    return static_cast<uint16_t>(
        ((TX_HW_FIFO_DEPTH * (53U + 64U) * 1000U) + baudrate - 1U) / baudrate);
}

uint8_t BxCanTransceiver::receiveInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < NUMBER_OF_TRANSCEIVERS && fpTransceivers[transceiverIndex] != nullptr)
    {
        // Accept all frames into the software queue - per-listener filtering
        // is done by notifyListeners() in receiveTask().
        return fpTransceivers[transceiverIndex]->fDevice.receiveISR(nullptr);
    }
    return 0U;
}

void BxCanTransceiver::transmitInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < NUMBER_OF_TRANSCEIVERS && fpTransceivers[transceiverIndex] != nullptr)
    {
        BxCanTransceiver* self = fpTransceivers[transceiverIndex];
        self->fDevice.transmitISR();

        // The bxCAN TX ISR does not report WHICH mailbox completed, so any
        // completion advances the listener job queue. With TXFP=1 requests
        // complete in submission order, but a fire-and-forget write()
        // submitted after a listener write() can complete first and trigger
        // the canFrameSent callback one frame early. Accepted: the listener
        // frame is already in a mailbox and completes within the hardware
        // queue drain time covered by getHwQueueTimeout().
        if (!self->fTxQueue.empty())
        {
            self->canFrameSentCallback();
        }
    }
}

void BxCanTransceiver::cyclicTask()
{
    // Check bus-off state
    if (fDevice.isBusOff())
    {
        if (getState() == State::OPEN)
        {
            setState(State::MUTED);
        }
    }
    else if (getState() == State::MUTED && !fMuted)
    {
        setState(State::OPEN);
    }
}

void BxCanTransceiver::disableRxInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < NUMBER_OF_TRANSCEIVERS && fpTransceivers[transceiverIndex] != nullptr)
    {
        fpTransceivers[transceiverIndex]->fDevice.disableRxInterrupt();
    }
}

void BxCanTransceiver::enableRxInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < NUMBER_OF_TRANSCEIVERS && fpTransceivers[transceiverIndex] != nullptr)
    {
        fpTransceivers[transceiverIndex]->fDevice.enableRxInterrupt();
    }
}

void BxCanTransceiver::receiveTask()
{
    // Mask the RX interrupt so receiveISR() cannot append to the queue
    // between getRxCount() and clearRxQueue() - frames enqueued in that
    // window would be dropped unread by the head advance.
    fDevice.disableRxInterrupt();

    uint8_t count = fDevice.getRxCount();
    for (uint8_t i = 0U; i < count; i++)
    {
        notifyListeners(fDevice.getRxFrame(i));
    }
    fDevice.clearRxQueue();

    // Re-enable RX FIFO interrupt after draining queue
    fDevice.enableRxInterrupt();
}

void BxCanTransceiver::canFrameSentCallback() { ::async::execute(fContext, _canFrameSent); }

void BxCanTransceiver::canFrameSentAsyncCallback()
{
    if (!fTxQueue.empty())
    {
        // If transceiver is no longer OPEN (bus-off, muted, closed), drop
        // all queued TX jobs without calling listeners.
        if (getState() != State::OPEN)
        {
            fTxQueue.clear();
            return;
        }

        TxJobWithCallback& job                 = fTxQueue.front();
        // Copy the frame by value: pop_front() below destroys this queue slot,
        // so a reference into it would dangle. The listener is a reference to
        // an external object and stays valid past the pop.
        ::can::CANFrame const frame            = job._frame;
        ::can::ICANFrameSentListener& listener = job._listener;
        fTxQueue.pop_front();

        bool sendAgain = false;
        if (!fTxQueue.empty())
        {
            if (getState() == State::OPEN)
            {
                sendAgain = true;
            }
            else
            {
                fTxQueue.clear();
            }
        }

        listener.canFrameSent(frame);
        notifyRegisteredSentListener(frame);

        if (sendAgain)
        {
            ::can::CANFrame const& nextFrame = fTxQueue.front()._frame;
            if (fDevice.transmit(nextFrame))
            {
                // Wait for next TX ISR
                return;
            }
            // HW queue full - no ISR will retrigger, clear remaining
            fTxQueue.clear();
            notifyRegisteredSentListener(nextFrame);
        }
    }
}

} // namespace bios
