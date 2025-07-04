// Copyright 2024 Accenture.

#include "inputManager/DigitalInputTester.h"

#include "inputManager/DigitalInput.h"
#include "util/format/SharedStringWriter.h"

namespace bios
{
DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(DigitalInputTester, "in", "inputs console")
COMMAND_GROUP_COMMAND(1U, "all", "print all inputs, verbose output")
COMMAND_GROUP_COMMAND(2U, "get", "get input value (param: <input-id>)")
COMMAND_GROUP_COMMAND(3U, "allx", "all inputs")
DEFINE_COMMAND_GROUP_GET_INFO_END

void DigitalInputTester::executeCommand(::util::command::CommandContext& context, uint8_t const idx)
{
    switch (static_cast<int32_t>(idx))
    {
        case 1: // "all"
        {
            ::util::format::SharedStringWriter out(context);
            (void)out.printf(
                "All digital inputs: %d \r\n", DigitalInput::TOTAL_NUMBER_OF_DIGITAL_INPUTS);
            for (uint16_t i = 0U; i < DigitalInput::TOTAL_NUMBER_OF_DIGITAL_INPUTS; ++i)
            {
                bool temp;
                bsp::BspReturnCode const ret
                    = DigitalInput::get(static_cast<DigitalInput::DigitalInputId>(i), temp);

                (void)out.printf(
                    "%s%d : %s -> ", (i < 10U) ? " " : "", i, DigitalInput::getName(i));
                if (ret == bsp::BSP_OK)
                {
                    (void)out.printf("%d\r\n", temp);
                }
                else
                {
                    (void)out.printf("not_ok\r\n");
                }
            }
        }
        break;
        case 2: // "get"
        {
            uint16_t const inputNo = context.scanIntToken<uint16_t>();
            (void)context.check(inputNo < DigitalInput::TOTAL_NUMBER_OF_DIGITAL_INPUTS);
            if (context.checkEol())
            {
                ::util::format::SharedStringWriter out(context);
                (void)out.printf("\r\n dInput %d -> ", inputNo);
                bool temp;
                bsp::BspReturnCode const ret
                    = DigitalInput::get(static_cast<DigitalInput::DigitalInputId>(inputNo), temp);
                if (ret == bsp::BSP_OK)
                {
                    (void)out.printf("%d \r\n", temp);
                }
                else
                {
                    (void)out.printf("nOk \r\n");
                }
            }
        }
        break;
        case 3: // "allx"
        {
            ::util::format::SharedStringWriter out(context);
            (void)out.printf("#");
            for (uint16_t i = 0U; i < DigitalInput::TOTAL_NUMBER_OF_DIGITAL_INPUTS; ++i)
            {
                bool temp;
                bsp::BspReturnCode const ret
                    = DigitalInput::get(static_cast<DigitalInput::DigitalInputId>(i), temp);
                if (ret == bsp::BSP_OK)
                {
                    (void)out.printf("%d ", temp);
                }
                else
                {
                    (void)out.printf("nOk ");
                }
            }
            (void)out.printf("$");
        }
        break;
        default:
        {
            break;
        }
    }
}

} /* namespace bios */
