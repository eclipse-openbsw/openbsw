/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "platform/estdint.h"

namespace uds
{
class RoutineControlOptionParser
{
public:
    static uint8_t getLogicalBlockNumberLength(uint8_t lengthFormatIdentifier);
    static uint8_t getMemoryAddressLength(uint8_t lengthFormatIdentifier);
    static uint8_t getMemorySizeLength(uint8_t lengthFormatIdentifier);
    static uint32_t parseParameter(uint8_t const* buffer, uint8_t length);
};

} // namespace uds
