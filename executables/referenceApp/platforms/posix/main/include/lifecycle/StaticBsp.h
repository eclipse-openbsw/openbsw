// Copyright 2024 Accenture.

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
