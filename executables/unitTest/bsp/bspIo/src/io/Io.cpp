// Copyright 2024 Accenture.

#include "io/Io.h"

using namespace bsp;

namespace bios
{
#undef BSP_IO_PIN_CONFIGURATION
#define BSP_IO_PIN_CONFIGURATION 1
#include "bsp/io/io/ioConfiguration.h"

BspReturnCode Io::setDefaultConfiguration(uint16_t io) { return BSP_NOT_SUPPORTED; }

uint32_t Io::getPinNumber(PinId pinId) { return 0; }

bool Io::getPin(uint16_t io) { return false; }

BspReturnCode Io::setPin(uint16_t io, bool vol) { return BSP_NOT_SUPPORTED; }

BspReturnCode Io::resetConfig(uint16_t io) { return BSP_NOT_SUPPORTED; }

bsp::BspReturnCode Io::safe(uint16_t io, bool lock) { return BSP_NOT_SUPPORTED; }

} /* namespace bios */
