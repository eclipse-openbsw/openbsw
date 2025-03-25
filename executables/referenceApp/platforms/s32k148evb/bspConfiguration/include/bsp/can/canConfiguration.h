// Copyright 2024 Accenture.

#pragma once

#include "can/FlexCANDevice.h"
#include "io/Io.h"

bios::FlexCANDevice::Config const Can0Config
    = {0x40024000U,
       can::AbstractCANTransceiver::BAUDRATE_HIGHSPEED,
       // PRESDIV                PSEG1        PSEG2        PROPSEG
       (0U << 24) | (0U << 22) | (4U << 19) | (2U << 16) | (6U << 0),
       bios::Io::canTx,
       bios::Io::canRx,
       14,
       2,
       15,
       0,
       0};
