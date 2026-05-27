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

#include "adc/AnalogInputScaleImplementation.h"

namespace bios
{
typedef bios::AnalogInputScaleImplementation<uint32_t, uint16_t> AnalogInputScale;

}
