// Copyright 2025 Accenture.

#include "ip/to_str.h"

#include <cstdio>

namespace ip
{
::etl::span<char> to_str(IPAddress const& ipAddr, ::etl::span<char> const& buffer)
{
    IPAddress::Family const family = addressFamilyOf(ipAddr);
    size_t const bufferSize        = buffer.size();

    int printfResult = 0;

    if ((IPAddress::IPV4 == family) && (bufferSize >= IP4_MAX_STRING_LENGTH))
    {
        printfResult = ::std::snprintf(
            buffer.data(),
            bufferSize,
            "%d.%d.%d.%d",
            ipAddr.raw[internal::RAW_IP4_IDX].bytes[0U],
            ipAddr.raw[internal::RAW_IP4_IDX].bytes[1U],
            ipAddr.raw[internal::RAW_IP4_IDX].bytes[2U],
            ipAddr.raw[internal::RAW_IP4_IDX].bytes[3U]);
    }
    else if ((IPAddress::IPV6 == family) && (bufferSize >= IP6_MAX_STRING_LENGTH))
    {
        printfResult = ::std::snprintf(
            buffer.data(),
            bufferSize,
            "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x",
            ipAddr.raw[0U].bytes[0U],
            ipAddr.raw[0U].bytes[1U],
            ipAddr.raw[0U].bytes[2U],
            ipAddr.raw[0U].bytes[3U],
            ipAddr.raw[1U].bytes[0U],
            ipAddr.raw[1U].bytes[1U],
            ipAddr.raw[1U].bytes[2U],
            ipAddr.raw[1U].bytes[3U],
            ipAddr.raw[2U].bytes[0U],
            ipAddr.raw[2U].bytes[1U],
            ipAddr.raw[2U].bytes[2U],
            ipAddr.raw[2U].bytes[3U],
            ipAddr.raw[3U].bytes[0U],
            ipAddr.raw[3U].bytes[1U],
            ipAddr.raw[3U].bytes[2U],
            ipAddr.raw[3U].bytes[3U]);
    }
    else
    {
        return {};
    }

    // The posix specification allows a possible negative code although for sprintf there's no
    // condition in which any error should be triggered. That's why it's not checked here.
    return ::etl::span<char>(buffer.data(), static_cast<size_t>(printfResult));
}

::etl::span<char> to_str(IPEndpoint const& endp, ::etl::span<char> const& buffer)
{
    IPAddress const& ipAddr        = endp.getAddress();
    IPAddress::Family const family = addressFamilyOf(ipAddr);
    size_t const bufferSize        = buffer.size();

    int printfResult = 0;

    if ((IPAddress::IPV4 == family) && (bufferSize >= IP4_ENDPOINT_MAX_STRING_LENGTH))
    {
        printfResult = ::std::snprintf(
            buffer.data(),
            bufferSize,
            "%d.%d.%d.%d:%d",
            ipAddr.raw[internal::RAW_IP4_IDX].bytes[0U],
            ipAddr.raw[internal::RAW_IP4_IDX].bytes[1U],
            ipAddr.raw[internal::RAW_IP4_IDX].bytes[2U],
            ipAddr.raw[internal::RAW_IP4_IDX].bytes[3U],
            endp.getPort());
    }
    else if ((IPAddress::IPV6 == family) && (bufferSize >= IP6_ENDPOINT_MAX_STRING_LENGTH))
    {
        printfResult = ::std::snprintf(
            buffer.data(),
            bufferSize,
            "[%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x]:%d",
            ipAddr.raw[0U].bytes[0U],
            ipAddr.raw[0U].bytes[1U],
            ipAddr.raw[0U].bytes[2U],
            ipAddr.raw[0U].bytes[3U],
            ipAddr.raw[1U].bytes[0U],
            ipAddr.raw[1U].bytes[1U],
            ipAddr.raw[1U].bytes[2U],
            ipAddr.raw[1U].bytes[3U],
            ipAddr.raw[2U].bytes[0U],
            ipAddr.raw[2U].bytes[1U],
            ipAddr.raw[2U].bytes[2U],
            ipAddr.raw[2U].bytes[3U],
            ipAddr.raw[3U].bytes[0U],
            ipAddr.raw[3U].bytes[1U],
            ipAddr.raw[3U].bytes[2U],
            ipAddr.raw[3U].bytes[3U],
            endp.getPort());
    }
    else
    {
        return {};
    }

    // The posix specification allows a possible negative code although for sprintf there's no
    // condition in which any error should be triggered. That's why it's not checked here.
    return ::etl::span<char>(buffer.data(), static_cast<size_t>(printfResult));
}

} /* namespace ip */
