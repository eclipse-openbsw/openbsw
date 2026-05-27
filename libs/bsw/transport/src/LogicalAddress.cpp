/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "transport/LogicalAddress.h"

#include <etl/algorithm.h>

namespace transport
{
namespace addressfinder
{
::etl::optional<LogicalAddress>
findBy2ByteAddress(::etl::span<LogicalAddress const> const& list, uint16_t const address)
{
    auto* const iter = etl::find_if(
        list.begin(),
        list.end(),
        [address](LogicalAddress const addr) -> bool { return addr.address2Byte == address; });
    if (iter != list.end())
    {
        return *iter;
    }
    return {};
}

::etl::optional<LogicalAddress>
findBy1ByteAddress(::etl::span<LogicalAddress const> const& list, uint16_t const address)
{
    auto* const iter = etl::find_if(
        list.begin(),
        list.end(),
        [address](LogicalAddress const addr) -> bool { return addr.address1Byte == address; });
    if (iter != list.end())
    {
        return *iter;
    }
    return {};
}
} // namespace addressfinder
} // namespace transport
