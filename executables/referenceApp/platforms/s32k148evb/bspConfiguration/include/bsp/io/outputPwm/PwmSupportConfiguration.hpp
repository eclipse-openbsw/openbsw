// Copyright 2024 Accenture.

#ifndef GUARD_AE978583_BA31_4073_8FD6_A46EB93177CF
#define GUARD_AE978583_BA31_4073_8FD6_A46EB93177CF

#include "ftmPwm/FtmECombinePwm.h"
#include "ftmPwm/FtmEPwm.h"
#include "ftmPwm/FtmModCombinePwm.h"
#include "io/Io.h"

namespace bios
{

// ftm4:1
tFtmEPwmConfiguration const EVAL_LED_RED_PWM_Configuration
    = {tFtm::PWM_EdgeAlignedetSet0, false, false, false, Io::EVAL_LED_RED, 0, 10000};

// ftm4:2
tFtmEPwmConfiguration const EVAL_LED_GREEN_PWM_Configuration
    = {tFtm::PWM_EdgeAlignedetSet0, false, false, false, Io::EVAL_LED_GREEN, 0, 10000};

// ftm4:3
tFtmEPwmConfiguration const EVAL_LED_BLUE_PWM_Configuration
    = {tFtm::PWM_EdgeAlignedetSet0, false, false, false, Io::EVAL_LED_BLUE, 0, 10000};

} // namespace bios

#endif // GUARD_AE978583_BA31_4073_8FD6_A46EB93177CF
