/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

/**
 * Contains interface IFilteredCANFrameSentListener.
 * \file    IFilteredCANFrameSentListener.h
 * \ingroup    canframes
 */
#pragma once

#include <etl/intrusive_forward_list.h>

namespace can
{
class CANFrame;
class IFilter;

/**
 * Interface for a filtered listener for sent CANFrames.
 *
 */
class IFilteredCANFrameSentListener : public ::etl::forward_link<0>
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

inline IFilteredCANFrameSentListener::IFilteredCANFrameSentListener() : ::etl::forward_link<0>() {}

} // namespace can
