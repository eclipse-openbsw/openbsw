/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#ifndef GUARD_70E33568_FE3A_4704_B9C1_8E5F6D6F1168
#define GUARD_70E33568_FE3A_4704_B9C1_8E5F6D6F1168

#include "someip/SdMessageConstants.h"

namespace someip
{
enum
{
    MINIMAL_MESSAGE_SIZE
    = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_MESSAGE_ID)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_REQUEST_ID)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_PROTOCOL_VERSION)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_INTERFACE_VERSION)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_MESSAGE_TYPE)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_RETURN_CODE)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_FLAGS)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_RESERVED)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_ENTRIES_ARRAY)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_LENGTH_OF_OPTIONS_ARRAY),
    IPV4_ENDPOINT_OPTION_SIZE
    = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_LENGTH)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_TYPE)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED1)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_IPV4ADDRESS)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED2)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PROTO)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PORT_NUMBER),
    IPV6_ENDPOINT_OPTION_SIZE
    = static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_LENGTH)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_TYPE)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED1)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_IPV6ADDRESS)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_RESERVED2)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PROTO)
      + static_cast<uint32_t>(MessageFieldSize::MESSAGE_FIELD_SIZE_ENDPOINT_OPTION_PORT_NUMBER)
};

}

#endif // GUARD_70E33568_FE3A_4704_B9C1_8E5F6D6F1168
