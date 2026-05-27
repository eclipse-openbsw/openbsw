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

#include "io/Io.h"
#include "outputManager/Output.h"

using namespace bios;

namespace enetphy
{

typedef struct
{
    Output::OutputId dataPin;
    Output::OutputId clockPin;
    Io::PinId mdioPin;
} PhyConfig;

constexpr PhyConfig tja1101config = {Output::TJA_MDIO, Output::TJA_MDC, Io::TJA_MDIO};
} // namespace enetphy
