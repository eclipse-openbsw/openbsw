/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/ChannelNames.h"

#include "routing/Header.h"

#include <blob/ConfigType.h>

namespace routing
{
::etl::span<uint8_t const> name(ChannelNames const& config, uint8_t const channelId)
{
    if (config.offsets.empty())
    {
        return {};
    }

    auto const namesCount = config.offsets.size() - 1U;
    if (channelId >= namesCount)
    {
        return {};
    }

    auto const currentOffset = static_cast<size_t>(config.offsets[channelId]);
    auto const nextOffset
        = static_cast<size_t>(config.offsets[static_cast<size_t>(channelId) + 1U]);
    if ((nextOffset <= currentOffset) || (config.names.size() < nextOffset))
    {
        return {};
    }

    return config.names.subspan(currentOffset, nextOffset - currentOffset);
}

bool load(::etl::span<uint8_t const> mem, ChannelNames& channelNames)
{
    if (mem.empty())
    {
        return false;
    }

    ::routing::Header const header = ::routing::load(mem);

    if (header.configType != ::blob::ConfigType::CHANNEL_NAMES)
    {
        return false;
    }

    if (mem.size() < sizeof(::etl::be_uint16_t))
    {
        return false;
    }

    auto const namesCount = static_cast<size_t>(mem.take<::etl::be_uint16_t const>());
    if (mem.size() < (namesCount + 1U) * sizeof(::etl::be_uint16_t))
    {
        return false;
    }

    channelNames.offsets = mem.take<::etl::be_uint16_t const>(namesCount + 1U);

    if (mem.size() < channelNames.offsets[namesCount])
    {
        return false;
    }

    channelNames.names = mem.take<uint8_t const>(channelNames.offsets[namesCount]);

    return true;
}
} // namespace routing
