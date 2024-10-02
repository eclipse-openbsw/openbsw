// Copyright 2024 Accenture.

/**
 * Contains interface ICANFrameListener.
 * \file ICANFrameListener.h
 * \ingroup framemgmt
 */
#ifndef GUARD_3991843A_AE0E_493A_8A61_E4A780891238
#define GUARD_3991843A_AE0E_493A_8A61_E4A780891238

#include <estd/forward_list.h>

namespace can
{
class CANFrame;
class IFilter;

/**
 * CANFrameListener interface
 *
 *
 * An ICANFrameListener subclass is a class interested in the reception
 * of CANFrames. Therefore it needs to register at an ICanTransceiver.
 */
class ICANFrameListener : public ::estd::forward_list_node<ICANFrameListener>
{
public:
    ICANFrameListener();
    ICANFrameListener(ICANFrameListener const&)            = delete;
    ICANFrameListener& operator=(ICANFrameListener const&) = delete;

    /**
     * This method notifies the listener of a CANFrame reception.
     */
    virtual void frameReceived(CANFrame const& canFrame) = 0;

    /**
     * Returns the ICANFrameListeners filter.
     */
    virtual IFilter& getFilter() = 0;
};

/*
 * inline implementation
 */
inline ICANFrameListener::ICANFrameListener() : ::estd::forward_list_node<ICANFrameListener>() {}

} // namespace can

#endif // GUARD_3991843A_AE0E_493A_8A61_E4A780891238
