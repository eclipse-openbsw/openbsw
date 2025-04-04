// Copyright 2024 Accenture.

#pragma once

#include "ftmPwm/FtmCentralAlignedCombinePwm.h"
#include "ftmPwm/FtmECombinePwm.h"
#include "ftmPwm/FtmEPwm.h"
#include "ftmPwm/FtmModCombinePwm.h"
#include "outputPwm/OutputPwm.h"

namespace bios
{
class PwmSupport : public OutputPwm::IDynamicPwmClient
{
public:
    PwmSupport(tFtm& ftm4);
    void init();
    void start();
    void stop();
    void shutdown();
    virtual bsp::BspReturnCode setDuty(uint16_t chan, uint16_t duty, bool immediateUpdate);
    virtual bsp::BspReturnCode setPeriod(uint16_t chan, uint16_t period);

private:
    // < hw channel, resolution >
    FtmEPwm<1, 100> eval_led_red_pwm;
    FtmEPwm<2, 100> eval_led_green_pwm;
    FtmEPwm<3, 100> eval_led_blue_pwm;
};

} // namespace bios
