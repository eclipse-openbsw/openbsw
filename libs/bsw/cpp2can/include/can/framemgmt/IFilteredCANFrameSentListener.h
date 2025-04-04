// Copyright 2024 Accenture.

/**
 * Contains interface IFilteredCANFrameSentListener.
 * \file    IFilteredCANFrameSentListener.h
 * \ingroup    canframes
 */
#pragma once

#include <estd/forward_list.h>

namespace can
{
class CANFrame;
class IFilter;

/**
 * Interface for a filtered listener for sent CANFrames.
 *
 */
class IFilteredCANFrameSentListener
: public ::estd::forward_list_node<IFilteredCANFrameSentListener>
{
protected:
    IFilteredCANFrameSentListener();

public:
    IFilteredCANFrameSentListener(IFilteredCANFrameSentListener const&)            = delete;
    IFilteredCANFrameSentListener& operator=(IFilteredCANFrameSentListener const&) = delete;
    /**
     * \warning
     * This callback is called from ISR context!
     */
    virtual void canFrameSent(CANFrame const& frame)                               = 0;

    /**
     * Returns the filter for which frames the listeners wants to be notified.
     */
    virtual IFilter& getFilter() = 0;
};

inline IFilteredCANFrameSentListener::IFilteredCANFrameSentListener()
: ::estd::forward_list_node<IFilteredCANFrameSentListener>()
{}

} // namespace can
