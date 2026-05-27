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

enum outputPwm
{
    _pwmStaticStart = 0,
    dummyPwm        = _pwmStaticStart,
    _pwmStaticEnd,
    _pwmDynamicStart = _pwmStaticEnd

    ,
    EVAL_LED_GREEN_PWM = _pwmDynamicStart,
    EVAL_LED_BLUE_PWM

    ,
    _pwmDynamicMark,
    _pwmChannelMax   = _pwmDynamicMark,
    PORT_UNAVAILABLE = _pwmChannelMax
};
