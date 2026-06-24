/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "routing/channelId.h"

#include <etl/span.h>

namespace routing
{
static constexpr uint8_t NUM_PDU_TRANSPORT_CHANNELS = 1U;
static constexpr uint8_t NUM_CAN_CHANNELS           = sizeof(::routing::canChannelIds);
static constexpr uint8_t NUM_FLEXRAY_CHANNELS       = sizeof(::routing::frChannelIds);
static constexpr uint8_t NUM_CHANNELS
    = NUM_CAN_CHANNELS + NUM_FLEXRAY_CHANNELS + NUM_PDU_TRANSPORT_CHANNELS;

} // namespace routing
