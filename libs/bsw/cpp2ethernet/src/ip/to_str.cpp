/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "ip/to_str.h"

#include <etl/format.h>

namespace ip
{
namespace
{
/**
 * Formats into buffer, reserving one character for the terminating NUL (etl::format_to_n does
 * not terminate the output itself) and returns the span of the written characters.
 */
template<typename... Args>
::etl::span<char>
formatTo(::etl::span<char> const& buffer, ::etl::format_string<Args...> fmt, Args&&... args)
{
    char* const begin = buffer.data();
    char* const end
        = ::etl::format_to_n(begin, buffer.size() - 1U, fmt, ::etl::forward<Args>(args)...);
    *end = '\0';
    return ::etl::span<char>(begin, static_cast<size_t>(end - begin));
}

} // namespace

::etl::span<char> to_str(IPAddress const& ipAddr, ::etl::span<char> const& buffer)
{
    IPAddress::Family const family = addressFamilyOf(ipAddr);
    size_t const bufferSize        = buffer.size();

    if ((IPAddress::IPV4 == family) && (bufferSize >= IP4_MAX_STRING_LENGTH))
    {
        return formatTo(
            buffer,
            "{}.{}.{}.{}",
            static_cast<unsigned>(ipAddr.raw[internal::RAW_IP4_IDX + 0U]),
            static_cast<unsigned>(ipAddr.raw[internal::RAW_IP4_IDX + 1U]),
            static_cast<unsigned>(ipAddr.raw[internal::RAW_IP4_IDX + 2U]),
            static_cast<unsigned>(ipAddr.raw[internal::RAW_IP4_IDX + 3U]));
    }
#ifdef PLATFORM_SUPPORT_IPV6
    if ((IPAddress::IPV6 == family) && (bufferSize >= IP6_MAX_STRING_LENGTH))
    {
        return formatTo(
            buffer,
            "{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:"
            "02x}:{:02x}{:02x}",
            static_cast<unsigned>(ipAddr.raw[0U]),
            static_cast<unsigned>(ipAddr.raw[1U]),
            static_cast<unsigned>(ipAddr.raw[2U]),
            static_cast<unsigned>(ipAddr.raw[3U]),
            static_cast<unsigned>(ipAddr.raw[4U]),
            static_cast<unsigned>(ipAddr.raw[5U]),
            static_cast<unsigned>(ipAddr.raw[6U]),
            static_cast<unsigned>(ipAddr.raw[7U]),
            static_cast<unsigned>(ipAddr.raw[8U]),
            static_cast<unsigned>(ipAddr.raw[9U]),
            static_cast<unsigned>(ipAddr.raw[10U]),
            static_cast<unsigned>(ipAddr.raw[11U]),
            static_cast<unsigned>(ipAddr.raw[12U]),
            static_cast<unsigned>(ipAddr.raw[13U]),
            static_cast<unsigned>(ipAddr.raw[14U]),
            static_cast<unsigned>(ipAddr.raw[15U]));
    }
#endif
    return {};
}

::etl::span<char> to_str(IPEndpoint const& endp, ::etl::span<char> const& buffer)
{
    IPAddress const& ipAddr        = endp.getAddress();
    IPAddress::Family const family = addressFamilyOf(ipAddr);
    size_t const bufferSize        = buffer.size();

    if ((IPAddress::IPV4 == family) && (bufferSize >= IP4_ENDPOINT_MAX_STRING_LENGTH))
    {
        return formatTo(
            buffer,
            "{}.{}.{}.{}:{}",
            static_cast<unsigned>(ipAddr.raw[internal::RAW_IP4_IDX + 0U]),
            static_cast<unsigned>(ipAddr.raw[internal::RAW_IP4_IDX + 1U]),
            static_cast<unsigned>(ipAddr.raw[internal::RAW_IP4_IDX + 2U]),
            static_cast<unsigned>(ipAddr.raw[internal::RAW_IP4_IDX + 3U]),
            static_cast<unsigned>(endp.getPort()));
    }
#ifdef PLATFORM_SUPPORT_IPV6
    if ((IPAddress::IPV6 == family) && (bufferSize >= IP6_ENDPOINT_MAX_STRING_LENGTH))
    {
        return formatTo(
            buffer,
            "[{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{:02x}:{:02x}{"
            ":02x}:{:02x}{:02x}]:{}",
            static_cast<unsigned>(ipAddr.raw[0U]),
            static_cast<unsigned>(ipAddr.raw[1U]),
            static_cast<unsigned>(ipAddr.raw[2U]),
            static_cast<unsigned>(ipAddr.raw[3U]),
            static_cast<unsigned>(ipAddr.raw[4U]),
            static_cast<unsigned>(ipAddr.raw[5U]),
            static_cast<unsigned>(ipAddr.raw[6U]),
            static_cast<unsigned>(ipAddr.raw[7U]),
            static_cast<unsigned>(ipAddr.raw[8U]),
            static_cast<unsigned>(ipAddr.raw[9U]),
            static_cast<unsigned>(ipAddr.raw[10U]),
            static_cast<unsigned>(ipAddr.raw[11U]),
            static_cast<unsigned>(ipAddr.raw[12U]),
            static_cast<unsigned>(ipAddr.raw[13U]),
            static_cast<unsigned>(ipAddr.raw[14U]),
            static_cast<unsigned>(ipAddr.raw[15U]),
            static_cast<unsigned>(endp.getPort()));
    }
#endif
    return {};
}

} /* namespace ip */
