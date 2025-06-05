// Copyright 2024 Accenture.

/**
 * Contains interface ICANFrameListener.
 * \file ICANFrameListener.h
 * \ingroup framemgmt
 */
#pragma once

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
     * Returns the ICANFrameListener's filter.
     */
    virtual IFilter& getFilter() = 0;
};

/*
 * inline implementation
 */
inline ICANFrameListener::ICANFrameListener() : ::estd::forward_list_node<ICANFrameListener>() {}

} // namespace can
