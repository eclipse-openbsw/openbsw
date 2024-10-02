// Copyright 2024 Accenture.

#include "can/transceiver/AbstractCANTransceiver.h"

#include <bsp/timer/SystemTimer.h>
#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>

#include <platform/config.h>

#include <cstring>

namespace can
{
using interrupts::SuspendResumeAllInterruptsScopedLock;

AbstractCANTransceiver::AbstractCANTransceiver(uint8_t const busId)
: _filter()
, _listeners()
, _sentListener(nullptr)
, _sentListeners()
, _baudrate(0U)
, _state(State::CLOSED)
, _busId(busId)
, _stateListener(nullptr)
, _transceiverState(ICANTransceiverStateListener::CANTransceiverState::ACTIVE)
{}

void AbstractCANTransceiver::addCANFrameListener(ICANFrameListener& listener)
{
    ESR_UNUSED const SuspendResumeAllInterruptsScopedLock lock;
    if (!_listeners.contains_node(listener))
    {
        _listeners.push_back(listener);
        listener.getFilter().acceptMerger(_filter);
    }
}

void AbstractCANTransceiver::addVIPCANFrameListener(ICANFrameListener& listener)
{
    ESR_UNUSED const SuspendResumeAllInterruptsScopedLock lock;
    if (!_listeners.contains_node(listener))
    {
        _listeners.push_front(listener);
        listener.getFilter().acceptMerger(_filter);
    }
}

void AbstractCANTransceiver::removeCANFrameListener(ICANFrameListener& listener)
{
    ESR_UNUSED const SuspendResumeAllInterruptsScopedLock lock;
    _listeners.remove(listener);
}

void AbstractCANTransceiver::addCANFrameSentListener(IFilteredCANFrameSentListener& listener)
{
    ESR_UNUSED const SuspendResumeAllInterruptsScopedLock lock;
    _sentListeners.push_front(listener);
}

void AbstractCANTransceiver::removeCANFrameSentListener(IFilteredCANFrameSentListener& listener)
{
    ESR_UNUSED const SuspendResumeAllInterruptsScopedLock lock;
    _sentListeners.remove(listener);
}

void AbstractCANTransceiver::notifyListeners(CANFrame const& frame)
{
    if (_state == State::CLOSED)
    {
        return; // don't receive messages in state CLOSED
    }

    for (auto& listener : _listeners)
    {
        if (listener.getFilter().match(frame.getId()))
        {
            listener.frameReceived(frame);
        }
    }
}

void AbstractCANTransceiver::notifySentListeners(can::CANFrame const& frame)
{
    if (_sentListeners.empty())
    {
        return;
    }

    const_cast<can::CANFrame&>(frame).setTimestamp(getSystemTimeUs32Bit());
    for (auto& sentListener : _sentListeners)
    {
        sentListener.canFrameSent(frame);
    }
}

} // namespace can
