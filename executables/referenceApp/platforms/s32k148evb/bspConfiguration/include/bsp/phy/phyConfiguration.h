// Copyright 2025 Accenture.

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
