// Copyright 2024 Accenture.

#include "tester/AnalogTester.h"

#include "bsp/adc/AnalogInput.h"
#include "bsp/adc/AnalogInputScale.h"
#include "util/format/SharedStringWriter.h"

namespace bios
{

using namespace ::util::command;
using namespace ::util::format;
using namespace ::util::stream;

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(AnalogTester, "adc", "Adc Console")
COMMAND_GROUP_COMMAND(1U, "all", "Print All Adc")
COMMAND_GROUP_COMMAND(2U, "get", "startChannel ChannelNr [stopChannel ChannelNr]")
COMMAND_GROUP_COMMAND(3U, "allx", "all scaled adc")
DEFINE_COMMAND_GROUP_GET_INFO_END;

void AnalogTester::executeCommand(CommandContext& context, uint8_t const idx)
{
    switch (idx)
    {
        case 1: // "all"
        {
            SharedStringWriter out(context);
            static_cast<void>(out.printf("All Adc Inputs: %d\n\r", AnalogInput::AdcCountAll));
            for (uint16_t i = 0U; i < AnalogInput::AdcCountAll; ++i)
            {
                AnalogInputScale::scaleReturnType temp   = 0U;
                AnalogInputScale::scaleReturnType scaled = 0U;
                static_cast<void>(AnalogInput::get(i, temp));
                bsp::BspReturnCode const ret = bios::AnalogInputScale::get(i, scaled);
                static_cast<void>(
                    out.printf("%s%d : %s -> ", (i < 10U) ? " " : "", i, AnalogInput::getName(i)));
                if (ret == bsp::BSP_OK)
                {
                    static_cast<void>(out.printf(": %d (raw)  - %d mV (scaled)\n\r", temp, scaled));
                }
                else
                {
                    static_cast<void>(out.printf("nOk \n\r"));
                }
            }
        }
        break;
        case 2: // "get"
        {
            uint32_t const startChannel = context.scanIntToken<uint32_t>();
            uint32_t stopChannel        = startChannel;
            static_cast<void>(context.check(startChannel < AnalogInput::AdcCountAll));

            if (context.hasToken())
            {
                stopChannel = context.scanIntToken<uint32_t>();
                static_cast<void>(context.check(stopChannel < AnalogInput::AdcCountAll));
            }

            if (context.checkEol())
            {
                SharedStringWriter out(context);
                for (uint32_t ix = startChannel; ix <= stopChannel; ++ix)
                {
                    static_cast<void>(out.printf(
                        "Adc Channel %d : %s -> ",
                        ix,
                        AnalogInput::getName(static_cast<uint16_t>(ix))));
                    AnalogInputScale::scaleReturnType temp   = 0U;
                    AnalogInputScale::scaleReturnType scaled = 0U;
                    static_cast<void>(AnalogInput::get(static_cast<uint16_t>(ix), temp));
                    bsp::BspReturnCode const ret
                        = bios::AnalogInputScale::get(static_cast<uint16_t>(ix), scaled);
                    if (ret == bsp::BSP_OK)
                    {
                        static_cast<void>(
                            out.printf(": %d (raw)  - %d mV (scaled)\n\r", temp, scaled));
                    }
                    else
                    {
                        static_cast<void>(out.printf("nOk \n\r"));
                    }
                }
            }
        }
        break;
        case 3: // "allx"
        {
            SharedStringWriter out(context);
            static_cast<void>(out.printf("#"));
            for (uint16_t i = 0U; i < AnalogInput::AdcCountAll; ++i)
            {
                uint32_t scaled              = 0U;
                bsp::BspReturnCode const ret = bios::AnalogInputScale::get(i, scaled);
                if (ret == bsp::BSP_OK)
                {
                    static_cast<void>(out.printf("%d ", scaled));
                }
                else
                {
                    static_cast<void>(out.printf("nOk "));
                }
            }
            static_cast<void>(out.printf("$"));
        }
        break;
        default:
        {
            break;
        }
    }
}

} // namespace bios
