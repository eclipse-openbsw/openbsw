/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include "bsp/eeprom/IEepromDriver.h"
#include "eeprom/EepromDriver.h"

class StaticBsp
{
public:
    StaticBsp() {}

    void init();

    eeprom::IEepromDriver& getEepromDriver() { return _eepromDriver; }

private:
    ::eeprom::EepromDriver _eepromDriver;
};
