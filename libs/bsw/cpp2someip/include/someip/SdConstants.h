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

enum class SdConstants : uint16_t
{
    SD_ENTRIES_LENGTH_FIELD_LENGTH = 4U,
    SD_ENTRIES_LENGTH_OFFSET       = 4U,
    SD_ENTRY_LENGTH                = 16U,
    SD_ENTRY_TYPE_OFFSET           = 0U,
    SD_EVENTGROUP_OFFSET           = 14U,
    SD_FLAGS_OFFSET                = 0U,
    SD_HEADER_LENGTH               = 4U,
    SD_INSTANCE_ID_OFFSET          = 6U,
    SD_IP4_OPTION_LENGTH           = 12U,
    SD_IP6_OPTION_LENGTH           = 24U,
    SD_MAJOR_VERSION_OFFSET        = 8U,
    SD_MESSAGE_ID_FIELD_LENGTH     = 4U,
    SD_MINOR_VERSION_OFFSET        = 12U,
    SD_OPTIONS_1_INDEX_OFFSET      = 1U,
    SD_OPTIONS_2_INDEX_OFFSET      = 2U,
    SD_OPTIONS_LENGTH_FIELD_LENGTH = 4U,
    SD_OPTIONS_NUM_OFFSET          = 3U,
    SD_OPTION_ADDRESS_OFFSET       = 4U,
    SD_OPTION_LENGTH_FIELD_LENGTH  = 2U,
    SD_OPTION_LENGTH_OFFSET        = 0U,
    SD_OPTION_TYPE_OFFSET          = 2U,
    SD_RESERVED_FIELD_LENGTH       = 2U,
    SD_RESERVED_OFFSET             = 12U,
    SD_SERVICE_ID_OFFSET           = 4U,
    SD_TTL_OFFSET                  = 9U
};

enum class SdFlags : uint8_t
{
    SD_FLAG_REBOOT  = 0x80U,
    SD_FLAG_UNICAST = 0x40U
};

constexpr uint32_t SD_DEFAULT_INITIAL_DELAY          = 0U;
constexpr uint32_t SD_DEFAULT_REPETITIONS_BASE_DELAY = 30U;
constexpr uint32_t SD_DEFAULT_REPETITIONS_MAX        = 4U;
constexpr uint32_t SD_DEFAULT_CYCLIC_OFFER_DELAY     = 1000U;

} // namespace someip
