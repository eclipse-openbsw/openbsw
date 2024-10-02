// Copyright 2024 Accenture.

#include "outputPwm/OutputPwmTester.h"

#include "outputPwm/OutputPwm.h"

#include <util/format/SharedStringWriter.h>

namespace bios
{

using namespace ::util::command;
using namespace ::util::format;
using namespace ::util::stream;

// clang-format off

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(OutputPwmTester, "pwm", "Set PWM")
    COMMAND_GROUP_COMMAND(1,"all","print all channel numbers and names")
    COMMAND_GROUP_COMMAND(2,"set","channel[0-9,0xFF=all] duty[0=off|10000=100%]")
    COMMAND_GROUP_COMMAND(3,"setPeriod","channel[0-9] period[microsec]")
DEFINE_COMMAND_GROUP_GET_INFO_END

// clang-format on

OutputPwmTester::OutputPwmTester() {}

void OutputPwmTester::executeCommand(CommandContext& context, uint8_t idx)
{
    switch (idx)
    {
        case 1:
        {
            SharedStringWriter out(context);
            uint32_t chanId = 0;
            for (chanId = 0; chanId < OutputPwm::outputPwmNumberAll; ++chanId)
            {
                out.printf("\n %d : %s \r", chanId, OutputPwm::getName(chanId));
            }
            break;
        }
        case 2: // set
        {
            SharedStringWriter out(context);
            uint32_t chanId = context.scanIntToken<uint32_t>();

            if (chanId >= OutputPwm::outputPwmNumberAll)
            {
                if (chanId != 0xFF)
                {
                    return;
                }
            }
            uint32_t dutyCycle = context.scanIntToken<uint32_t>();
            if (dutyCycle > OutputPwm::MAX_DUTY_CYCLE)
            {
                dutyCycle = OutputPwm::MAX_DUTY_CYCLE;
            }
            if (chanId != 0xFF)
            {
                bsp::BspReturnCode ret = OutputPwm::setDuty(chanId, dutyCycle, false);
                if (ret == bsp::BSP_OK)
                {
                    out.printf(
                        "\n PWM channel %d (%s) set to 0x%x %% On\r",
                        chanId,
                        OutputPwm::getName(chanId),
                        dutyCycle);
                }
                else
                {
                    out.printf("nOk \n\r");
                }
            }
            else
            {
                // setDuty() for all channels
                for (chanId = 0; chanId < OutputPwm::outputPwmNumberAll; ++chanId)
                {
                    bsp::BspReturnCode ret = OutputPwm::setDuty(chanId, dutyCycle, false);
                    if (ret == bsp::BSP_OK)
                    {
                        out.printf(
                            "\n PWM channel %d (%s) set to 0x%x %% On\r",
                            chanId,
                            OutputPwm::getName(chanId),
                            dutyCycle);
                    }
                    else
                    {
                        out.printf("nOk \n\r");
                    }
                }
            }
        }
        break;
        case 3: // set period
        {
            SharedStringWriter out(context);
            uint32_t chanId = context.scanIntToken<uint32_t>();

            if (chanId >= OutputPwm::outputPwmNumberAll)
            {
                out.printf("nOk wrong channel\n\r");
            }
            uint32_t period = context.scanIntToken<uint32_t>();
            if (period > 0xffff)
            {
                period = 0xffff;
            }
            bsp::BspReturnCode ret = OutputPwm::setPeriod(chanId, period);
            if (ret == bsp::BSP_OK)
            {
                out.printf(
                    "\n PWM channel %d (%s) set to %d mksec On\r",
                    chanId,
                    OutputPwm::getName(chanId),
                    period);
            }
            else
            {
                out.printf("nOk \n\r");
            }
        }
        break;
        default: break;
    }
}

} // namespace bios
