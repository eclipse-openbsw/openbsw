/*******************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
// This is a generated file. Please do not edit it.

#pragma once

#include <cstdint>

#include "routing/channelId.h"

namespace routing
{
static constexpr uint8_t NUM_PDU_TRANSPORT_CHANNELS = 9U;
static constexpr uint8_t NUM_CAN_CHANNELS
    = static_cast<uint8_t>(sizeof(::routing::canChannelIds) / sizeof(::routing::canChannelIds[0]));
static constexpr uint8_t NUM_FLEXRAY_CHANNELS
    = static_cast<uint8_t>(sizeof(::routing::frChannelIds) / sizeof(::routing::frChannelIds[0]));
static constexpr uint8_t NUM_CHANNELS
    = NUM_CAN_CHANNELS + NUM_FLEXRAY_CHANNELS + NUM_PDU_TRANSPORT_CHANNELS;

} // namespace routing
