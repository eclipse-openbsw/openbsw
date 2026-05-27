/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <etl/optional.h>
#include <etl/span.h>

#include <array>

namespace transport
{

struct LogicalAddress
{
    uint16_t address2Byte;
    uint16_t address1Byte;
};

namespace addressfinder
{
::etl::optional<LogicalAddress>
findBy2ByteAddress(::etl::span<LogicalAddress const> const& list, uint16_t address);

::etl::optional<LogicalAddress>
findBy1ByteAddress(::etl::span<LogicalAddress const> const& list, uint16_t address);

inline bool is2ByteAddressIn(uint16_t const address, ::etl::span<LogicalAddress const> const& list)
{
    return findBy2ByteAddress(list, address).has_value();
}

inline bool is1ByteAddressIn(uint16_t const address, ::etl::span<LogicalAddress const> const& list)
{
    return findBy1ByteAddress(list, address).has_value();
}
} // namespace addressfinder

template<size_t N>
class LogicalAddressConverter
{
public:
    static uint16_t convert2ByteAddressTo1Byte(uint16_t const address)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wundefined-var-template"
        for (auto& l : TESTER_ADDRESS_LISTS)
#pragma GCC diagnostic pop
        {
            auto ret = addressfinder::findBy2ByteAddress(l, address);
            if (ret.has_value())
            {
                return ret->address1Byte;
            }
        }
        return address;
    }

    static uint16_t convert1ByteAddressTo2Byte(uint16_t const address)
    {
        for (auto& l : TESTER_ADDRESS_LISTS)
        {
            auto ret = addressfinder::findBy1ByteAddress(l, address);
            if (ret.has_value())
            {
                return ret->address2Byte;
            }
        }
        return address;
    }

    static etl::array<::etl::span<LogicalAddress const>, N> const TESTER_ADDRESS_LISTS;
};
} // namespace transport
