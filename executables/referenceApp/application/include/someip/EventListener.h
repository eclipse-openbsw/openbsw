/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "someip/IEventListener.h"
#include "someip/SomeIpParser.h"

class EventListener : public someip::IEventListener
{
public:
    EventListener()                                = default;
    EventListener& operator=(EventListener const&) = delete;
    EventListener(EventListener const&)            = delete;

    // someip::IEventListener
    void eventReceived(
        uint16_t serviceId,
        uint16_t eventId,
        uint16_t instanceId,
        uint8_t majorVersion,
        ::someip::SomeIpParser& parser) override;
};
