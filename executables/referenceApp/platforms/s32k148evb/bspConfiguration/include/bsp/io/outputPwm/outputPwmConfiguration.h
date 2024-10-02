// Copyright 2024 Accenture.

#ifndef GUARD_0559CCBA_AF28_45E6_BBD9_59A5553C8649
#define GUARD_0559CCBA_AF28_45E6_BBD9_59A5553C8649

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

#endif /* GUARD_0559CCBA_AF28_45E6_BBD9_59A5553C8649 */
