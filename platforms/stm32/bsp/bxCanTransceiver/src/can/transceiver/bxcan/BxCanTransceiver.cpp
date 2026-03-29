// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file BxCanTransceiver.cpp
 * \brief Implementation of the STM32 bxCAN transceiver.
 *
 * Design overview:
 * - Lifecycle follows CLOSED -> INITIALIZED -> OPEN -> MUTED -> CLOSED.
 * - TX path with listener uses an async callback chain:
 *     write(frame, listener) queues {listener, frame} into fTxQueue,
 *     transmits the first frame. TX ISR calls canFrameSentCallback()
 *     which defers to task context via async::execute. The async callback
 *     pops the queue, calls listener.canFrameSent(), transmits the next
 *     queued frame, and notifies registered sent listeners.
 * - TX path without listener (fire-and-forget): write() calls
 *     BxCanDevice::transmit() and notifies sent-listeners synchronously.
 * - RX path is interrupt-driven: receiveInterrupt() is called from the CAN RX
 *   ISR, which copies frames into a software queue inside BxCanDevice. The ISR
 *   handler disables further RX interrupts to prevent re-entry. receiveTask()
 *   runs in async task context to drain the queue and re-enable the interrupt.
 * - Bus-off recovery is handled by cyclicTask(), which polls the peripheral
 *   status and transitions between OPEN and MUTED states accordingly.
 */

#include <can/transceiver/bxcan/BxCanTransceiver.h>

#include <can/canframes/ICANFrameSentListener.h>

namespace bios
{

BxCanTransceiver* BxCanTransceiver::fpTransceivers[3] = {nullptr, nullptr, nullptr};

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
    if (busId < 3U)
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

    // Auto-schedule bus-off polling (matching S32K scheduleAtFixedRate pattern)
    ::async::scheduleAtFixedRate(
        fContext, _cyclicTaskRunner, fCyclicTimeout, 10U, ::async::TimeUnit::MILLISECONDS);

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
        // Not the first in queue — will be sent from the TX ISR chain
        return ErrorCode::CAN_ERR_OK;
    }

    // First in queue — transmit now
    if (!fDevice.transmit(frame))
    {
        fTxQueue.pop_front();
        fOverrunCount++;
        notifyRegisteredSentListener(frame);
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }

    // Wait for TX ISR → canFrameSentCallback() → canFrameSentAsyncCallback()
    return ErrorCode::CAN_ERR_OK;
}

uint32_t BxCanTransceiver::getBaudrate() const { return 500000U; }

uint16_t BxCanTransceiver::getHwQueueTimeout() const { return 10U; }

uint8_t BxCanTransceiver::receiveInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        // Accept all frames into the software queue — per-listener filtering
        // is done by notifyListeners() in receiveTask(), matching the S32K
        // CanFlex2Transceiver pattern.
        return fpTransceivers[transceiverIndex]->fDevice.receiveISR(nullptr);
    }
    return 0U;
}

void BxCanTransceiver::transmitInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        BxCanTransceiver* self = fpTransceivers[transceiverIndex];
        self->fDevice.transmitISR();

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
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        fpTransceivers[transceiverIndex]->fDevice.disableRxInterrupt();
    }
}

void BxCanTransceiver::enableRxInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        fpTransceivers[transceiverIndex]->fDevice.enableRxInterrupt();
    }
}

void BxCanTransceiver::receiveTask()
{
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
        ::can::CANFrame const& frame           = job._frame;
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
            // HW queue full — no ISR will retrigger, clear remaining
            fTxQueue.clear();
            notifyRegisteredSentListener(nextFrame);
        }
    }
}

} // namespace bios
