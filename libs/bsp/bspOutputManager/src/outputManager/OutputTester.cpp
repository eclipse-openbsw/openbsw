// Copyright 2024 Accenture.

#include "outputManager/OutputTester.h"

#include "outputManager/Output.h"
#include "util/format/SharedStringWriter.h"

namespace bios
{
DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(OutputTester, "out", "outputs console")
COMMAND_GROUP_COMMAND(1U, "all", "print all outputs")
COMMAND_GROUP_COMMAND(2U, "set", "set output value (param: <output-id> [0|1])")
COMMAND_GROUP_COMMAND(3U, "get", "get output value (param: <output-id>)")
DEFINE_COMMAND_GROUP_GET_INFO_END

void OutputTester::executeCommand(::util::command::CommandContext& context, uint8_t const idx)
{
    switch (static_cast<int32_t>(idx))
    {
        case 1: // "all"
        {
            ::util::format::SharedStringWriter out(context);
            (void)out.printf("All outputs:\r\n");
            for (int32_t i = 0; i < Output::TOTAL_NUMBER_OF_OUTPUTS; i++)
            {
                (void)out.printf("%s%d : %s -> ", (i < 10) ? " " : "", i, Output::getName(i));
                bool t;
                bsp::BspReturnCode const ret = Output::get(static_cast<Output::OutputId>(i), t);
                if (ret == bsp::BSP_OK)
                {
                    (void)out.printf("%d\r\n", t);
                }
                else
                {
                    (void)out.printf("not_ok\r\n");
                }
            }
        }
        break;
        case 2: // "set"
        {
            uint32_t const outputNo = context.scanIntToken<uint32_t>();
            uint32_t vol            = context.scanIntToken<uint32_t>();
            (void)context.check(
                static_cast<Output::OutputId>(outputNo) < Output::TOTAL_NUMBER_OF_OUTPUTS);
            if (vol > 0U)
            {
                vol = 1U;
            }

            if (context.checkEol())
            {
                ::util::format::SharedStringWriter out(context);
                (void)out.printf("%d : %s -> ", outputNo, Output::getName(outputNo));
                if (Output::set(static_cast<Output::OutputId>(outputNo), vol)
                    == static_cast<uint32_t>(bsp::BSP_OK))
                {
                    (void)out.printf("ok \r\n");
                }
                else
                {
                    (void)out.printf("Nok \r\n");
                }
            }
            break;
        }
        case 3: // "get"
        {
            ::util::format::SharedStringWriter out(context);
            uint32_t const outputNo = context.scanIntToken<uint32_t>();
            (void)context.check(
                static_cast<Output::OutputId>(outputNo) < Output::TOTAL_NUMBER_OF_OUTPUTS);
            (void)out.printf("%d : %s -> ", outputNo, Output::getName(outputNo));
            bool t;
            bsp::BspReturnCode const ret = Output::get(static_cast<Output::OutputId>(outputNo), t);
            if (ret == bsp::BSP_OK)
            {
                (void)out.printf("%d\r\n", t);
            }
            else
            {
                (void)out.printf("Nok \r\n");
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

} /* namespace bios */
