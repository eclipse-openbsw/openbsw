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

#include <etl/span.h>
#include <etl/unaligned_type.h>

#include <platform/estdint.h>

namespace routing
{

struct ChannelNames
{
    ::etl::span<::etl::be_uint16_t const> offsets;
    ::etl::span<uint8_t const> names;
};

/**
 * Returns the name of the channel with the given ID, or an empty span if not found.
 */
::etl::span<uint8_t const> name(::routing::ChannelNames const& config, uint8_t channelId);

/**
 * Loads channel names from mem.
 * Returns true on success and false on error.
 * On failure, channel names may be partially modified and must not be used.
 */
bool load(::etl::span<uint8_t const> mem, ChannelNames& channelNames);

} // namespace routing
