// Copyright 2024 Accenture.

#include "app/CanDemoListener.h"

#include <can/CanLogger.h>

namespace can
{

using ::util::logger::CAN;
using ::util::logger::Logger;

CanDemoListener::CanDemoListener(::can::ICanTransceiver* canTransceiver)
: _canFilter(), _canTransceiver(canTransceiver)
{}

::can::IFilter& CanDemoListener::getFilter() { return _canFilter; }

void CanDemoListener::frameReceived(::can::CANFrame const& frame)
{
    Logger::debug(
        CAN,
        "[CanDemoListener] received CAN frame, id=0x%X, length=%d",
        (int)frame.getId(),
        (int)frame.getPayloadLength());

    if (_canTransceiver != nullptr)
    {
        ::can::CANFrame newFrame(frame.getId() + 1, frame.getPayload(), frame.getPayloadLength());
        auto result = _canTransceiver->write(newFrame);
        Logger::debug(CAN, "[CanDemoListener] response queued, result = %d", (int)result);
    }
}

void CanDemoListener::canFrameSent(::can::CANFrame const& frame)
{
    Logger::debug(
        CAN,
        "[CanDemoListener] CAN frame sent, id=0x%X, length=%d",
        (int)frame.getId(),
        (int)frame.getPayloadLength());
}

void CanDemoListener::run()
{
    _canFilter.add(0x123);
    _canFilter.add(0x124);

    if (_canTransceiver != nullptr)
    {
        _canTransceiver->addCANFrameListener(*this);
        _canTransceiver->addCANFrameSentListener(*this);
    }
}

void CanDemoListener::shutdown()
{
    if (_canTransceiver != nullptr)
    {
        _canTransceiver->removeCANFrameListener(*this);
        _canTransceiver->removeCANFrameSentListener(*this);
    }
}

} // namespace can
