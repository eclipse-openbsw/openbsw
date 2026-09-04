/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <cstdint>

namespace someip
{
// clang-format off
enum class MessageFieldSize : uint32_t
{
    // related to endpoint option in order of appearance in memory
    // {{
    MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_LENGTH      = 2U,
    MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_TYPE        = 1U,
    MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED1   = 1U,
    MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_IPV4ADDRESS = 4U,
    MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_IPV6ADDRESS = 16U,
    MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED2   = 1U,
    MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PROTO       = 1U,
    MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PORT_NUMBER = 2U,
    MESSAGE_FIELD_SIZE_ENDPOINT_IPV4_OPTION_LENGTH
    = MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_LENGTH + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_TYPE
      + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED1
      + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_IPV4ADDRESS
      + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED2 + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PROTO
      + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PORT_NUMBER,
    MESSAGE_FIELD_SIZE_ENDPOINT_IPV6_OPTION_LENGTH
    = MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_LENGTH + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_TYPE
      + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED1
      + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_IPV6ADDRESS
      + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED2 + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PROTO
      + MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PORT_NUMBER,
    // }}

    // message generic fields in order of appearance in memory
    // {{
    MESSAGE_FIELD_SIZE_MESSAGE_ID              = 4U,
    MESSAGE_FIELD_SIZE_LENGTH                  = 4U,
    MESSAGE_FIELD_SIZE_REQUEST_ID              = 4U,
    MESSAGE_FIELD_SIZE_PROTOCOL_VERSION        = 1U,
    MESSAGE_FIELD_SIZE_INTERFACE_VERSION       = 1U,
    MESSAGE_FIELD_SIZE_MESSAGE_TYPE            = 1U,
    MESSAGE_FIELD_SIZE_RETURN_CODE             = 1U,
    MESSAGE_FIELD_SIZE_FLAGS                   = 1U,
    MESSAGE_FIELD_SIZE_RESERVED                = 3U,
    MESSAGE_FIELD_SIZE_LENGTH_OF_ENTRIES_ARRAY = 4U,
    MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY = 4U,
    // }}

    // related to entries in order of appearance in memory
    // {{
    MESSAGE_FIELD_SIZE_ENTRY                   = 16U, // entry size (unconditional)
    MESSAGE_FIELD_SIZE_ENTRY_TYPE              = 1U,  // size of entry type
    MESSAGE_FIELD_SIZE_ENTRY_INDEX_1ST_OPTIONS = 1U,  // size of 1st index
    MESSAGE_FIELD_SIZE_ENTRY_INDEX_2ND_OPTIONS = 1U,  // size of 2nd index
    MESSAGE_FIELD_SIZE_ENTRY_SERVICE_ID        = 2U,  // size of service ID
    MESSAGE_FIELD_SIZE_ENTRY_INSTANCE_ID       = 2U,  // size of instance ID
    MESSAGE_FIELD_SIZE_ENTRY_MAJOR_VERSION     = 1U,  // size of major version
    MESSAGE_FIELD_SIZE_ENTRY_TTL               = 3U,  // size of TTL
    MESSAGE_FIELD_SIZE_ENTRY_MINOR_VERSION     = 4U,  // size of minor version
    MESSAGE_FIELD_SIZE_ENTRY_RESERVED_COUNTER
    = 2U, // size of reserved (12 bits) + counter (4 bits) in case of event group entry
    MESSAGE_FIELD_SIZE_ENTRY_EVENT_GROUP_ID = 2U // size of event group ID
    // }}
};

// clang-format on

} // namespace someip
