/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/EventListener.h"

#include "someip/SomeIpConstants.h"
#include "someip/logger.h"

using ::util::logger::SOMEIP;

void EventListener::eventReceived(
    ::someip::service_id::type serviceId,
    uint16_t eventId,
    ::someip::instance_id::type,
    ::someip::major_version::type,
    ::someip::SomeIpParser& parser)
{
    INFO_LOG(
        SOMEIP,
        "event received: serviceId=0x%x eventId=0x%x bytesAvailable=%u",
        serviceId,
        eventId,
        parser.bytesAvailable());
}
