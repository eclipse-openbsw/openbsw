/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/RoutineControlOptionParser.h"

#include <etl/unaligned_type.h>

namespace uds
{
uint8_t
RoutineControlOptionParser::getLogicalBlockNumberLength(uint8_t const lengthFormatIdentifier)
{
    return (lengthFormatIdentifier & 0xF0U) >> 4U;
}

uint8_t RoutineControlOptionParser::getMemoryAddressLength(uint8_t const lengthFormatIdentifier)
{
    return (lengthFormatIdentifier & 0xF0U) >> 4U;
}

uint8_t RoutineControlOptionParser::getMemorySizeLength(uint8_t const lengthFormatIdentifier)
{
    return (lengthFormatIdentifier & 0x0FU);
}

uint32_t
RoutineControlOptionParser::parseParameter(uint8_t const* const buffer, uint8_t const length)
{
    switch (length)
    {
        case 1U:
        {
            return static_cast<uint32_t>(*buffer);
        }
        case 2U:
        {
            return static_cast<uint32_t>(::etl::be_uint16_t(buffer));
        }
        case 3U:
        {
            return (static_cast<uint32_t>(*buffer) << 16)
                   | static_cast<uint32_t>(::etl::be_uint16_t(buffer + 1));
        }
        case 4U:
        {
            return ::etl::be_uint32_t(buffer);
        }
        default:
        {
            return 0U;
        }
    }
}

} // namespace uds
