// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file FdCanTransceiver.cpp
 * \brief Implementation of the STM32 FDCAN transceiver.
 *
 * Design overview:
 * - Lifecycle follows CLOSED -> INITIALIZED -> OPEN -> MUTED -> CLOSED.
 * - TX without listener: synchronous transmit + notifySentListeners.
 * - TX with listener: queued async pattern matching S32K CanFlex2Transceiver:
 *   write() queues {listener, frame} into fTxQueue, TX ISR defers to task
 *   context via async::execute, canFrameSentAsyncCallback() pops queue,
 *   notifies listener, and sends next queued frame.
 * - RX path is interrupt-driven: receiveInterrupt() is called from the CAN RX
 *   ISR, which copies frames into a software queue inside FdCanDevice. The ISR
 *   handler disables further RX interrupts to prevent re-entry. receiveTask()
 *   runs in async task context to drain the queue and re-enable the interrupt.
 * - Bus-off recovery is handled by cyclicTask(), which polls the peripheral
 *   status and transitions between OPEN and MUTED states accordingly.
 */

#include <can/transceiver/fdcan/FdCanTransceiver.h>

#include <can/canframes/ICANFrameSentListener.h>

namespace bios
{

FdCanTransceiver* FdCanTransceiver::fpTransceivers[3] = {nullptr, nullptr, nullptr};

FdCanTransceiver::FdCanTransceiver(
    ::async::ContextType context, uint8_t busId, FdCanDevice::Config const& devConfig)
: AbstractCANTransceiver(busId)
, fDevice(devConfig)
, fContext(context)
, fCyclicTimeout()
, _canFrameSent(::async::Function::CallType::
                    create<FdCanTransceiver, &FdCanTransceiver::canFrameSentAsyncCallback>(*this))
, fTxQueue()
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

    fDevice.init();
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
        fDevice.init();
    }

    fDevice.start();
    setState(State::OPEN);
    fMuted = false;
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

    if (!fDevice.transmit(frame))
    {
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
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }

    bool const wasEmpty = fTxQueue.empty();
    fTxQueue.emplace_back(listener, frame);

    if (!wasEmpty)
    {
        // Next frame will be sent from TX ISR callback chain.
        return ErrorCode::CAN_ERR_OK;
    }

    // We are the first sender — transmit with TX event enabled.
    fDevice.fTxEventEnabled = true;
    if (!fDevice.transmit(frame))
    {
        fDevice.fTxEventEnabled = false;
        fTxQueue.pop_front();
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }
    fDevice.fTxEventEnabled = false;

    // Wait until TX interrupt triggers canFrameSentCallback().
    return ErrorCode::CAN_ERR_OK;
}

uint32_t FdCanTransceiver::getBaudrate() const { return 500000U; }

uint16_t FdCanTransceiver::getHwQueueTimeout() const { return 10U; }

// Software ISR filter bitmap: accept only CAN IDs matching registered listeners.
// 256 bytes = 2048 bits covers all standard 11-bit CAN IDs (0x000-0x7FF).
// Bit set = accept, bit clear = reject.
static uint8_t sIsrFilterBitField[256] = {};
static bool sIsrFilterInitialized = false;

uint8_t FdCanTransceiver::receiveInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        // Initialize filter bitmap once: accept 0x7E0 (diagnostic request)
        if (!sIsrFilterInitialized)
        {
            // Set bit for 0x7E0: byte 252, bit 0
            uint32_t const id = 0x7E0U;
            sIsrFilterBitField[id / 8U] |= (1U << (id % 8U));
            sIsrFilterInitialized = true;
        }
        return fpTransceivers[transceiverIndex]->fDevice.receiveISR(sIsrFilterBitField);
    }
    return 0U;
}

void FdCanTransceiver::transmitInterrupt(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        FdCanTransceiver* self = fpTransceivers[transceiverIndex];
        self->fDevice.transmitISR();

        // Always dispatch callback when the TX queue has a pending frame.
        // CRITICAL: Check fTxQueue EVERY TIME TC fires, not just once.
        // When the demo sends 0x558 (no listener) and DoCAN sends 0x7E8 (with listener)
        // near-simultaneously, both completions produce TC. If the demo's TC fires first,
        // fTxQueue might already have the DoCAN frame waiting. We must dispatch.
        // The 50ms poll provides a safety net, but the ISR dispatch gives <1ms latency.
        if (!self->fTxQueue.empty())
        {
            self->canFrameSentCallback();
        }
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
            fDevice.fTxEventEnabled      = true;
            if (!fDevice.transmit(frame))
            {
                fTxQueue.clear();
            }
            fDevice.fTxEventEnabled = false;
        }
    }
}

void FdCanTransceiver::pollTxCallback(uint8_t transceiverIndex)
{
    if (transceiverIndex < 3U && fpTransceivers[transceiverIndex] != nullptr)
    {
        FdCanTransceiver* self = fpTransceivers[transceiverIndex];
        if (!self->fTxQueue.empty())
        {
            self->canFrameSentAsyncCallback();
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

} // close bios namespace temporarily
extern volatile uint32_t g_rxTaskCount;
extern volatile uint32_t g_rx7E0PreNotify;
extern volatile uint32_t g_rxNon7E0;
extern volatile uint32_t g_rxLastNon7E0Id;
// Store first 16 non-0x7E0 IDs for debugging
volatile uint32_t g_rxBadIds[16] = {};
volatile uint8_t  g_rxBadIdIdx = 0U;
namespace bios { // reopen

void FdCanTransceiver::receiveTask()
{
    uint8_t count = fDevice.getRxCount();
    ::g_rxTaskCount += count;
    for (uint8_t i = 0U; i < count; i++)
    {
        ::can::CANFrame const& frame = fDevice.getRxFrame(i);
        uint32_t id = frame.getId();
        if (id == 0x7E0U)
        {
            ::g_rx7E0PreNotify++;
        }
        else
        {
            ::g_rxNon7E0++;
            ::g_rxLastNon7E0Id = id;
            if (g_rxBadIdIdx < 16U) { g_rxBadIds[g_rxBadIdIdx++] = id; }
        }
        notifyListeners(frame);
    }
    fDevice.clearRxQueue();
    // Re-enable RX FIFO interrupt after draining software queue
    fDevice.enableRxInterrupt();
}

} // namespace bios
