/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/Header.h"

#include <blob/Config.h>
#include <etl/unaligned_type.h>

namespace routing
{
bool load(::etl::span<uint8_t const>& mem, Header& header)
{
    if (mem.size() < sizeof(::blob::Config::Header))
    {
        return false;
    }

    auto configHeaderMem = mem.take<uint8_t const>(sizeof(::blob::Config::Header));

    uint32_t const configTypeValue = configHeaderMem.take<::etl::be_uint32_t const>();

    if ((configTypeValue != static_cast<uint32_t>(::blob::ConfigType::ROUTING))
        && (configTypeValue != static_cast<uint32_t>(::blob::ConfigType::RX_ADAPTER))
        && (configTypeValue != static_cast<uint32_t>(::blob::ConfigType::TX_ADAPTER))
        && (configTypeValue != static_cast<uint32_t>(::blob::ConfigType::CHANNEL))
        && (configTypeValue != static_cast<uint32_t>(::blob::ConfigType::CHANNEL_NAMES))
        && (configTypeValue != static_cast<uint32_t>(::blob::ConfigType::META)))
    {
        return false;
    }

    header.configType = static_cast<::blob::ConfigType>(configTypeValue);

    uint16_t const channelTypeValue = configHeaderMem.take<::etl::be_uint16_t const>();

    // unused
    (void)configHeaderMem.advance(sizeof(uint8_t));

    auto const channelId = configHeaderMem.take<uint8_t const>();

    if ((header.configType == ::blob::ConfigType::CHANNEL)
        || (header.configType == ::blob::ConfigType::RX_ADAPTER)
        || (header.configType == ::blob::ConfigType::TX_ADAPTER))
    {
        if ((channelTypeValue != static_cast<uint32_t>(::routing::ChannelType::PDU_TRANSPORT))
            && (channelTypeValue != static_cast<uint32_t>(::routing::ChannelType::CAN))
            && (channelTypeValue != static_cast<uint32_t>(::routing::ChannelType::FR)))
        {
            return false;
        }

        header.channelType = static_cast<::routing::ChannelType>(channelTypeValue);
        header.channelId   = channelId;
    }

    return true;
}
} // namespace routing
