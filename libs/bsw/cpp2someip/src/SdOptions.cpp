/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/SdOptions.h"
#include "someip/SdConstants.h"

#include "someip/endian_helpers.h"

namespace someip
{
bool SdOptions::init(::etl::span<uint8_t const> const& payload, size_t const entriesLength)
{
    size_t const optionsLengthOffset = 8U + entriesLength;
    if ((optionsLengthOffset + static_cast<uint16_t>(SdConstants::SD_OPTIONS_LENGTH_FIELD_LENGTH))
        > payload.size())
    {
        return false;
    }

    auto const optionsLength = ::someip::endian::read_be<uint32_t>(&payload[optionsLengthOffset]);
    if ((optionsLengthOffset + static_cast<uint16_t>(SdConstants::SD_OPTIONS_LENGTH_FIELD_LENGTH)
         + optionsLength)
        > payload.size())
    {
        return false;
    }

    size_t const optionsPosition
        = optionsLengthOffset + static_cast<uint16_t>(SdConstants::SD_OPTIONS_LENGTH_FIELD_LENGTH);

    if (optionsPosition == payload.size())
    {
        _optionsData = decltype(_optionsData)();
    }
    else
    {
        _optionsData = payload.subspan(optionsPosition, optionsLength);
    }

    return true;
}

void SdOptions::readIndexValues(::etl::span<uint8_t const> const& entry)
{
    _options1Index = entry[static_cast<uint16_t>(SdConstants::SD_OPTIONS_1_INDEX_OFFSET)];
    _options2Index = entry[static_cast<uint16_t>(SdConstants::SD_OPTIONS_2_INDEX_OFFSET)];
    _options1Num   = entry[static_cast<uint16_t>(SdConstants::SD_OPTIONS_NUM_OFFSET)]
                   >> static_cast<uint8_t>(4U);
    _options2Num = entry[static_cast<uint16_t>(SdConstants::SD_OPTIONS_NUM_OFFSET)]
                   & static_cast<uint8_t>(0x0FU);
}

} // namespace someip
