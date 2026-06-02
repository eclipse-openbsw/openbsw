/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <can/transceiver/fdcan/FdCanTransceiver.h>

#include <can/canframes/ICANFrameSentListener.h>

namespace bios
{

FdCanTransceiver* FdCanTransceiver::fpTransceivers[3] = {nullptr, nullptr, nullptr};

FdCanTransceiver::FdCanTransceiver(
    ::async::ContextType context, uint8_t busId, FdCanDevice::Config const& devConfig)
: AbstractCANTransceiver(busId)
, fDevice(
      devConfig,
      ::etl::delegate<void()>::create<FdCanTransceiver, &FdCanTransceiver::canFrameSentCallback>(
          *this))
, fContext(context)
, fCyclicTimeout()
, _cyclicTaskRunner(
      ::async::Function::CallType::create<FdCanTransceiver, &FdCanTransceiver::cyclicTask>(*this))
, _canFrameSent(::async::Function::CallType::
                    create<FdCanTransceiver, &FdCanTransceiver::canFrameSentAsyncCallback>(*this))
, fTxQueue()
, fOverrunCount(0U)
, fMuted(false)
{
    if (busId < 3U)
    {
        fpTransceivers[busId] = this;
    }
}

::can::ICanTransceiver::ErrorCode FdCanTransceiver::init()
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

::can::ICanTransceiver::ErrorCode FdCanTransceiver::open()
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

    // Auto-schedule bus-off polling.
    ::async::scheduleAtFixedRate(
        fContext, _cyclicTaskRunner, fCyclicTimeout, 10U, ::async::TimeUnit::MILLISECONDS);

    return ErrorCode::CAN_ERR_OK;
}

::can::ICanTransceiver::ErrorCode FdCanTransceiver::open(::can::CANFrame const& /* frame */)
{
    return open();
}

::can::ICanTransceiver::ErrorCode FdCanTransceiver::close()
{
    if (getState() == State::CLOSED)
    {
        return ErrorCode::CAN_ERR_OK;
    }

    fCyclicTimeout.cancel();
    fDevice.stop();
    setState(State::CLOSED);
    return ErrorCode::CAN_ERR_OK;
}

void FdCanTransceiver::shutdown() { close(); }

::can::ICanTransceiver::ErrorCode FdCanTransceiver::mute()
{
    if (getState() != State::OPEN)
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }

    fMuted = true;
    setState(State::MUTED);
    return ErrorCode::CAN_ERR_OK;
}

::can::ICanTransceiver::ErrorCode FdCanTransceiver::unmute()
{
    if (getState() != State::MUTED)
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }

    fMuted = false;
    setState(State::OPEN);
    return ErrorCode::CAN_ERR_OK;
}

::can::ICanTransceiver::ErrorCode FdCanTransceiver::write(::can::CANFrame const& frame)
{
    if (getState() != State::OPEN || fMuted)
    {
        return ErrorCode::CAN_ERR_TX_OFFLINE;
    }

    // Transmit without TX-complete interrupt; no deferred callback is needed.
    if (!fDevice.transmit(frame, false))
    {
        fOverrunCount++;
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }

    notifySentListeners(frame);
    return ErrorCode::CAN_ERR_OK;
}

::can::ICanTransceiver::ErrorCode
FdCanTransceiver::write(::can::CANFrame const& frame, ::can::ICANFrameSentListener& listener)
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
        // Next frame will be sent from TX ISR callback chain.
        return ErrorCode::CAN_ERR_OK;
    }

    // First sender - transmit with TX-complete interrupt enabled.
    // Device enables TCE, ISR will fire on completion and invoke callback delegate.
    if (!fDevice.transmit(frame, true))
    {
        fTxQueue.pop_front();
        fOverrunCount++;
        notifyRegisteredSentListener(frame);
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }

    // Wait until TX ISR -> fFrameSentCallback -> canFrameSentCallback().
    return ErrorCode::CAN_ERR_OK;
}

uint32_t FdCanTransceiver::getBaudrate() const { return fDevice.getBaudrate(); }

uint16_t FdCanTransceiver::getHwQueueTimeout() const
{
    // Time for the hardware TX FIFO to drain: TX_HW_FIFO_DEPTH frames x (53 bit
    // CAN overhead + 64 bit payload) at the configured bit rate, rounded up
    // to the next millisecond so the timeout is never 0 ms.
    uint32_t const baudrate = fDevice.getBaudrate();
    // NOLINTNEXTLINE(clang-analyzer-core.DivideZero): CAN baudrate can never be zero here.
    return static_cast<uint16_t>(
        ((TX_HW_FIFO_DEPTH * (53U + 64U) * 1000U) + baudrate - 1U) / baudrate);
}

uint8_t FdCanTransceiver::receiveInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        // Accept all frames at ISR level. Per-listener filtering happens in
        // notifyListeners() during receiveTask().
        return fpTransceivers[transceiverIndex]->fDevice.receiveISR(nullptr);
    }
    return 0U;
}

void FdCanTransceiver::transmitInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        // Device's transmitISR() disables TCE, clears TC, and invokes the
        // callback delegate (bound to canFrameSentCallback).
        fpTransceivers[transceiverIndex]->fDevice.transmitISR();
    }
}

void FdCanTransceiver::canFrameSentCallback() { ::async::execute(fContext, _canFrameSent); }

void FdCanTransceiver::canFrameSentAsyncCallback()
{
    if (!fTxQueue.empty())
    {
        bool sendAgain = false;
        {
            TxJobWithCallback& job                 = fTxQueue.front();
            ::can::CANFrame const& frame           = job._frame;
            ::can::ICANFrameSentListener& listener = job._listener;
            fTxQueue.pop_front();

            if (!fTxQueue.empty())
            {
                // Send again only if same precondition as for write() is satisfied.
                State const state = getState();
                if ((State::OPEN == state) || (State::INITIALIZED == state))
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
        }

        if (sendAgain)
        {
            ::can::CANFrame const& frame = fTxQueue.front()._frame;
            // Transmit next queued frame with TX-complete interrupt enabled.
            if (!fDevice.transmit(frame, true))
            {
                fTxQueue.clear();
            }
        }
    }
}

void FdCanTransceiver::disableRxInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        fpTransceivers[transceiverIndex]->fDevice.disableRxInterrupt();
    }
}

void FdCanTransceiver::enableRxInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        fpTransceivers[transceiverIndex]->fDevice.enableRxInterrupt();
    }
}

void FdCanTransceiver::cyclicTask()
{
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

void FdCanTransceiver::receiveTask()
{
    uint8_t count = fDevice.getRxCount();
    for (uint8_t i = 0U; i < count; i++)
    {
        notifyListeners(fDevice.getRxFrame(i));
    }
    fDevice.clearRxQueue();
    // Re-enable RX FIFO interrupt after draining software queue
    fDevice.enableRxInterrupt();
}

} // namespace bios
