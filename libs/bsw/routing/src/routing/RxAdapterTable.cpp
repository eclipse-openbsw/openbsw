/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/RxAdapterTable.h"

#include "routing/Header.h"
#include "routing/util.h"

#include <blob/Config.h>
#include <blob/util.h>

namespace routing
{

bool load(::etl::span<uint8_t const> mem, RxAdapterTable& table)
{
    if (mem.empty())
    {
        return false;
    }

    ::routing::Header header;
    if (!::routing::load(mem, header))
    {
        return false;
    }

    if (header.configType != ::blob::Config::Type::RX_ADAPTER)
    {
        return false;
    }

    if (mem.size() < sizeof(table.firstId))
    {
        return false;
    }

    table.firstId = mem.take<::etl::be_uint32_t const>();
    mem           = ::blob::loadColumn(table.messageIds, mem);
    mem           = ::blob::loadColumn(table.messageLengths, mem);
    mem           = ::blob::loadColumn(table.pduLengthsOffsets, mem);
    mem           = ::blob::loadColumn(table.pduLengths, mem);
    mem           = ::blob::loadColumn(table.pduOffsets, mem);

    return true;
}

} // namespace routing
