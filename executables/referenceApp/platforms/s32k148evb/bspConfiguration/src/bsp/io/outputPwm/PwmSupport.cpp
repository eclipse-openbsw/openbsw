// Copyright 2024 Accenture.

#include "bsp/io/outputPwm/PwmSupport.h"

#include "bsp/io/outputPwm/PwmSupportConfiguration.hpp"
#include "outputPwm/OutputPwm.h"

namespace bios
{
PwmSupport::PwmSupport(tFtm& ftm4)
: eval_led_red_pwm(ftm4, EVAL_LED_RED_PWM_Configuration)
, eval_led_green_pwm(ftm4, EVAL_LED_GREEN_PWM_Configuration)
, eval_led_blue_pwm(ftm4, EVAL_LED_BLUE_PWM_Configuration)
{}

void PwmSupport::init()
{
    (void)eval_led_red_pwm.init();
    (void)eval_led_green_pwm.init();
    (void)eval_led_blue_pwm.init();

    (void)OutputPwm::setDynamicClient(OutputPwm::EVAL_LED_RED_PWM, 0, this);
    (void)OutputPwm::setDynamicClient(OutputPwm::EVAL_LED_GREEN_PWM, 1, this);
    (void)OutputPwm::setDynamicClient(OutputPwm::EVAL_LED_BLUE_PWM, 2, this);
}

void PwmSupport::start()
{
    (void)eval_led_red_pwm.start();
    (void)eval_led_green_pwm.start();
    (void)eval_led_blue_pwm.start();
}

void PwmSupport::stop()
{
    (void)eval_led_red_pwm.stop();
    (void)eval_led_green_pwm.stop();
    (void)eval_led_blue_pwm.stop();
}

void PwmSupport::shutdown()
{
    stop();

    (void)OutputPwm::clrDynamicClient(OutputPwm::EVAL_LED_RED_PWM);
    (void)OutputPwm::clrDynamicClient(OutputPwm::EVAL_LED_GREEN_PWM);
    (void)OutputPwm::clrDynamicClient(OutputPwm::EVAL_LED_BLUE_PWM);
}

bsp::BspReturnCode PwmSupport::setDuty(uint16_t chan, uint16_t duty, bool /* immediateUpdate */)
{
    switch (chan)
    {
        case 0: eval_led_red_pwm.setDuty(eval_led_red_pwm.validDuty(duty)); break;
        case 1: eval_led_green_pwm.setDuty(eval_led_green_pwm.validDuty(duty)); break;
        case 2: eval_led_blue_pwm.setDuty(eval_led_blue_pwm.validDuty(duty)); break;

        default: return bsp::BSP_NOT_SUPPORTED;
    }
    return bsp::BSP_OK;
}

/*
 * Set period for specific channel. Unit: microseconds
 */
bsp::BspReturnCode PwmSupport::setPeriod(uint16_t chan, uint16_t period)
{
    switch (chan)
    {
        case 0:
        {
            (void)eval_led_red_pwm.setlPeriod(period);
            break;
        }
        case 1:
        {
            (void)eval_led_green_pwm.setlPeriod(period);
            break;
        }
        case 2:
        {
            (void)eval_led_blue_pwm.setlPeriod(period);
            break;
        }

        default: return bsp::BSP_NOT_SUPPORTED;
    }
    return bsp::BSP_OK;
}

} // namespace bios
