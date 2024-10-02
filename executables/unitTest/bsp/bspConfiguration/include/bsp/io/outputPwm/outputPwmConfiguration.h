// Copyright 2024 Accenture.

#ifndef GUARD_A2B3CCEF_2BCF_462F_A1BC_1770330C744D
#define GUARD_A2B3CCEF_2BCF_462F_A1BC_1770330C744D

// All outputs OutputPwm::
enum outputPwm
{
    _pwmStaticStart = 0,
    dummyPwm        = _pwmStaticStart,
    _pwmStaticEnd,
    _pwmDynamicStart = _pwmStaticEnd

    ,
    EVAL_LED_RED_PWM = _pwmDynamicStart,
    EVAL_LED_GREEN_PWM,
    EVAL_LED_BLUE_PWM

    ,
    _pwmDynamicMark,
    _pwmChannelMax   = _pwmDynamicMark,
    PORT_UNAVAILABLE = _pwmChannelMax

}; //*enum

#endif /* GUARD_A2B3CCEF_2BCF_462F_A1BC_1770330C744D */
