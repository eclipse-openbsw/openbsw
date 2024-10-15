// Copyright 2025 Accenture.

#pragma once

#include "ip/IPAddress.h"
#include "ip/IPEndpoint.h"

namespace ip
{
uint8_t const IP4_MAX_STRING_LENGTH = 16U;
uint8_t const IP6_MAX_STRING_LENGTH = 40U;
uint8_t const MAX_IP_STRING_LENGTH  = IP6_MAX_STRING_LENGTH;

uint8_t const IP4_ENDPOINT_MAX_STRING_LENGTH = 22U;
uint8_t const IP6_ENDPOINT_MAX_STRING_LENGTH = 48U;
uint8_t const MAX_ENDPOINT_STRING_LENGTH     = IP6_ENDPOINT_MAX_STRING_LENGTH;

::etl::span<char> to_str(IPAddress const& ipAddr, ::etl::span<char> const& buffer);

::etl::span<char> to_str(IPEndpoint const& endp, ::etl::span<char> const& buffer);

} /* namespace ip */
