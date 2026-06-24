/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/util.h"

#include <blob/Blob.h>
#include <etl/span.h>

namespace routing
{
::blob::Config config(
    ::etl::span<uint8_t const> const blob, ::blob::Config::Type const type, uint8_t const channelId)
{
    for (auto const config : ::blob::Blob(blob))
    {
        if (config.type != type)
        {
            continue;
        }

        auto data = config.data;
        ::routing::Header header;
        if (!::routing::load(data, header))
        {
            continue;
        }

        if (header.channelId != channelId)
        {
            continue;
        }

        return config;
    }

    return {};
}

} // namespace routing
