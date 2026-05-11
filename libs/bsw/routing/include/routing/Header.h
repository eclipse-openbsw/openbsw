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

/**
 * This file defines and provides functions to interact with the header common to all routing
 * configuration blobs.
 */

#include <blob/ConfigType.h>
#include <etl/limits.h>
#include <etl/span.h>

#include <platform/estdint.h>

namespace routing
{
static constexpr auto INVALID_CHANNEL_ID = ::etl::numeric_limits<uint8_t>::max();
static constexpr auto INVALID_DGA        = ::etl::numeric_limits<uint16_t>::max();

enum class ChannelType : uint32_t
{
    PDU_TRANSPORT = 0x00,
    CAN           = 0x01,
    FR            = 0x02,
    UNKNOWN       = 0x03
};

struct Header
{
    ::blob::ConfigType configType      = ::blob::ConfigType::UNKNOWN;
    ::routing::ChannelType channelType = ::routing::ChannelType::UNKNOWN;
    uint8_t channelId                  = INVALID_CHANNEL_ID;
};

/**
 * Loads and returns the header from mem, advancing mem past the header on success.
 * On error, a default (invalid) header is returned.
 */
Header load(::etl::span<uint8_t const>& mem);

} // namespace routing
