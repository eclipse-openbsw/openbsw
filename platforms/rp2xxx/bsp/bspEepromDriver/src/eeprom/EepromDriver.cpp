/********************************************************************************
 * Copyright (c) 2025 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "eeprom/EepromDriver.h"

#include <cache/cache.h>
#include <etl/memory.h>
#include <interrupts/suspendResumeAllInterrupts.h>

namespace eeprom
{

// EEPROM commands
EepromDriver::EepromDriver(EepromConfiguration const& configuration) : _configuration(configuration)
{}

bool EepromDriver::isOperationAllowed(
    uint32_t const address, uint8_t const* const buffer, uint32_t const length) const
{
    return true;
}

void EepromDriver::clearCommand() {}

bool EepromDriver::isInitialized() const { return true; }

__attribute__((aligned(32))) uint8_t launchCommandFromRAM() { return 0; }

bsp::BspReturnCode EepromDriver::launchCommand() { return bsp::BSP_OK; }

bsp::BspReturnCode EepromDriver::init()
{
    auto result = bsp::BSP_OK;

    return result;
}

bsp::BspReturnCode
EepromDriver::write(uint32_t const address, uint8_t const* const buffer, uint32_t const length)
{
    return bsp::BSP_OK;
}

bsp::BspReturnCode
EepromDriver::read(uint32_t const address, uint8_t* const buffer, uint32_t const length)
{
    return bsp::BSP_OK;
}

} // namespace eeprom
