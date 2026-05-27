/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "io/IoMock.h"

#include "io/Io.h"

namespace bios
{
bsp::BspReturnCode Io::setDefaultConfiguration(uint16_t io)
{
    return IoMock::instance().setDefaultConfiguration(io);
}

bool Io::getPin(uint16_t io) { return IoMock::instance().getPin(io); }

bsp::BspReturnCode Io::setPin(uint16_t io, bool vol) { return IoMock::instance().setPin(io, vol); }

uint32_t Io::getPinNumber(Io::PinId pinId) { return IoMock::instance().getPinNumber(pinId); }

bsp::BspReturnCode Io::resetConfig(uint16_t io) { return IoMock::instance().resetConfig(io); }

bsp::BspReturnCode Io::safe(uint16_t io, bool setLock)
{
    return IoMock::instance().safe(io, setLock);
}

bool Io::getPinIrq(uint16_t io) { return IoMock::instance().getPinIrq(io); }
} // namespace bios
