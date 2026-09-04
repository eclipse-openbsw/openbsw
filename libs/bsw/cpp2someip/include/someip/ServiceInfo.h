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

#include "someip/SomeIpConstants.h"

#include <etl/span.h>

// Contains helper for generated service info.

namespace someip
{
/**
 * Info on methods.
 */
struct MethodDetail
{
    uint16_t methodId;

    SomeIpMethodType methodType;
    SomeIpCallSemantic callSemantic;
};

/**
 * Info on events.
 */
struct EventDetail
{
    uint16_t eventId;

    SomeIpEventType eventType;
    ::etl::span<uint16_t const> eventGroupIds;
};

} // namespace someip
