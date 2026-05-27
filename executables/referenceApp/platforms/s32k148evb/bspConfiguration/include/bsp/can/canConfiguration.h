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
