// Copyright 2024 Accenture.

#ifndef GUARD_CCAF5DB9_0D99_4CDE_906F_6774F3E05D59
#define GUARD_CCAF5DB9_0D99_4CDE_906F_6774F3E05D59

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

#endif // GUARD_CCAF5DB9_0D99_4CDE_906F_6774F3E05D59
