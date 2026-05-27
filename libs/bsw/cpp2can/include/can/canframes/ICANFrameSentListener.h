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
 * Contains interface ICANFrameSentListener.
 * \file    ICANFrameSentListener.h
 * \ingroup    canframes
 */
#pragma once

namespace can
{
class CANFrame;

/**
 * Interface for listeners that need to be notified when a CANFrame has
 * been sent.
 *
 */
class ICANFrameSentListener
{
protected:
    ICANFrameSentListener() = default;

public:
    ICANFrameSentListener(ICANFrameSentListener const&)            = delete;
    ICANFrameSentListener& operator=(ICANFrameSentListener const&) = delete;
    /**
     * \warning
     * This callback is called from ISR context!
     */
    virtual void canFrameSent(CANFrame const& frame)               = 0;
};

} // namespace can
