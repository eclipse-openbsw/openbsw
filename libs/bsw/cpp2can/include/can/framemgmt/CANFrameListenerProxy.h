// Copyright 2024 Accenture.

#pragma once

#include "can/framemgmt/ICANFrameListener.h"

#include <estd/assert.h>

namespace can
{
class IFilter;

template<typename T>
class CANFrameListenerProxy : public T
{
public:
    CANFrameListenerProxy() : _listener(nullptr) {}

    explicit CANFrameListenerProxy(ICANFrameListener& listener) : _listener(&listener) {}

    void setCANFrameListener(ICANFrameListener& listener) { _listener = &listener; }

    IFilter& getFilter() { return T::getFilter(); }

    void frameReceived(CANFrame const& canFrame)
    {
        if (_listener != nullptr)
        {
            _listener->frameReceived(canFrame);
        }
    }

private:
    ICANFrameListener* _listener;
};

} // namespace can
