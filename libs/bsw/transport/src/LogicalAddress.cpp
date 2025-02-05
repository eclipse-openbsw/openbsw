// Copyright 2024 Accenture.

#include "transport/LogicalAddress.h"

#include <algorithm>

namespace transport
{
namespace addressfinder
{
::estd::optional<LogicalAddress>
findDoipAddressInSlice(uint16_t const address, ::estd::slice<LogicalAddress const> const& list)
{
    auto const iter = std::find_if(
        list.begin(),
        list.end(),
        [address](LogicalAddress const addr) -> bool { return addr.addressDoip == address; });
    if (iter != list.end())
    {
        return *iter;
    }
    return {};
}

::estd::optional<LogicalAddress>
find8BitAddressInSlice(uint16_t const address, ::estd::slice<LogicalAddress const> const& list)
{
    auto const iter = std::find_if(
        list.begin(),
        list.end(),
        [address](LogicalAddress const addr) -> bool { return addr.address8Bit == address; });
    if (iter != list.end())
    {
        return *iter;
    }
    return {};
}
} // namespace addressfinder
} // namespace transport
