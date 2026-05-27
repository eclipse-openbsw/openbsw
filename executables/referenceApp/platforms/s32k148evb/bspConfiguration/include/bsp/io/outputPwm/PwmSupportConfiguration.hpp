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

#include "ftmPwm/FtmECombinePwm.h"
#include "ftmPwm/FtmEPwm.h"
#include "ftmPwm/FtmModCombinePwm.h"
#include "io/Io.h"

namespace bios
{

// ftm4:2
tFtmEPwmConfiguration const EVAL_LED_GREEN_PWM_Configuration
    = {tFtm::PWM_EdgeAlignedetSet0, false, false, false, Io::EVAL_LED_GREEN, 0, 10000};

// ftm4:3
tFtmEPwmConfiguration const EVAL_LED_BLUE_PWM_Configuration
    = {tFtm::PWM_EdgeAlignedetSet0, false, false, false, Io::EVAL_LED_BLUE, 0, 10000};

} // namespace bios
