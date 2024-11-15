// Copyright 2024 Accenture.

#pragma once

#include <etl/optional.h>
#include <etl/span.h>

#include <array>

namespace transport
{

struct LogicalAddress
{
    uint16_t addressDoip;
    uint16_t address8Bit;
};

namespace addressfinder
{
::etl::optional<LogicalAddress>
findDoipAddressInSlice(uint16_t address, ::etl::span<LogicalAddress const> const& list);

::etl::optional<LogicalAddress>
find8BitAddressInSlice(uint16_t address, ::etl::span<LogicalAddress const> const& list);

inline bool isDoipAddressIn(uint16_t const address, ::etl::span<LogicalAddress const> const& list)
{
    return findDoipAddressInSlice(address, list).has_value();
}

inline bool is8BitAddressIn(uint16_t const address, ::etl::span<LogicalAddress const> const& list)
{
    return find8BitAddressInSlice(address, list).has_value();
}
} // namespace addressfinder

template<size_t N>
class LogicalAddressConverter
{
public:
    static uint16_t convertDoipAddressTo8Bit(uint16_t const address)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wundefined-var-template"
        for (auto& l : TESTER_ADDRESS_LISTS)
#pragma GCC diagnostic pop
        {
            auto ret = addressfinder::findDoipAddressInSlice(address, l);
            if (ret.has_value())
            {
                return ret->address8Bit;
            }
        }
        return address;
    }

    static uint16_t convert8BitAddressToDoip(uint16_t const address)
    {
        for (auto& l : TESTER_ADDRESS_LISTS)
        {
            auto ret = addressfinder::find8BitAddressInSlice(address, l);
            if (ret.has_value())
            {
                return ret->addressDoip;
            }
        }
        return address;
    }

    static etl::array<::etl::span<LogicalAddress const>, N> const TESTER_ADDRESS_LISTS;
};
} // namespace transport
