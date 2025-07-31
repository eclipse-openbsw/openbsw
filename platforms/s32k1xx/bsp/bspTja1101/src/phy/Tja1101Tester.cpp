// Copyright 2025 Accenture.

#include "phy/Tja1101Tester.h"

#include "commonDebug.h"
#include "phy/Tja1101.h"
#include "util/format/SharedStringWriter.h"

namespace enetphy
{

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(Tja1101Tester, "phy", "TJA1101 ENET PHY")
COMMAND_GROUP_COMMAND(CMD_ALL, "all", "print all registers")
COMMAND_GROUP_COMMAND(CMD_GET, "get", "get register from Tja1101")
COMMAND_GROUP_COMMAND(CMD_STRT, "start", "Set link UP and start communication")
COMMAND_GROUP_COMMAND(CMD_STOP, "stop", "Set Link DOWN")
DEFINE_COMMAND_GROUP_GET_INFO_END

Tja1101Tester::Tja1101Tester(Tja1101& ethernetPhyTja1101) : _ethernetPhyTja1101(ethernetPhyTja1101)
{}

void Tja1101Tester::executeCommand(::util::command::CommandContext& context, uint8_t const idx)
{
    ::util::format::SharedStringWriter writer(context);
    switch (idx)
    {
        case CMD_ALL:
        {
            uint16_t value = 0;
            for (int i = 0; i < Tja1101::NUMBER_OF_SMI_REGISTERS; i++)
            {
                if (_ethernetPhyTja1101.readMiimRegister(Tja1101::cRegisterAccess[i], value))
                {
                    writer.printf("\n\r Reg %2d , 0x%0004x ", Tja1101::cRegisterAccess[i], value);
                }
                else
                {
                    writer.printf(" OFFLINE\n\r");
                    break;
                }
            }
            break;
        }
        case CMD_GET:
        {
            uint8_t regNr = context.scanIntToken<uint8_t>();
            if (context.checkEol())
            {
                uint16_t regVal = 0;
                _ethernetPhyTja1101.readMiimRegister(regNr, regVal);
                writer.printf("\n\r Reg %d , 0x%0004x ", regNr, regVal);
            }
            break;
        }
        case CMD_STRT:
        {
            _ethernetPhyTja1101.start();
            break;
        }
        case CMD_STOP:
        {
            _ethernetPhyTja1101.stop();
            break;
        }
        default:
        {
            break;
        }
    }
}

} // namespace enetphy
