// Copyright 2025 Accenture.

#include "safety/console/SafetyCommand.h"

#include <io/Io.h>
#include <safeUtils/SafetyLogger.h>

#include <cstdio>

extern uint32_t __MPU_BSS_START[];

namespace
{

enum Id
{
    ID_MPU,
    ID_WDG,
    ID_PRT,
    ID_DST,
    ID_EST
};

}

namespace safety
{

using ::util::logger::Logger;
using ::util::logger::SAFETY;

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(SafetyCommand, "safety", "Commands to test safety features")
COMMAND_GROUP_COMMAND(ID_MPU, "mpu", "Write to protected memory (enforce MPU violation)")
COMMAND_GROUP_COMMAND(ID_WDG, "wdg", "Enter infinite loop (enforce watchdog reset)")
COMMAND_GROUP_COMMAND(ID_PRT, "prt", "Write to protected register (has no effect)")
COMMAND_GROUP_COMMAND(ID_DST, "dst", "Disable safety test pin (enter safe state)")
COMMAND_GROUP_COMMAND(ID_EST, "est", "Re-enable safety test pin (leave safe state)")
DEFINE_COMMAND_GROUP_GET_INFO_END

void SafetyCommand::executeCommand(::util::command::CommandContext&, uint8_t idx)
{
    switch (idx)
    {
        case ID_MPU:
        {
            // Using printf, because a regular logger call would not finish before reset.
            printf("Write to protected memory to trigger a hard fault due to MPU violation\n");
            uint32_t* start_bss = reinterpret_cast<uint32_t*>(__MPU_BSS_START);
            *start_bss          = 0x12345678;
            break;
        }
        case ID_WDG:
        {
            // Using printf, because a regular logger call would not finish before reset.
            printf("Enter infinite loop to trigger watchdog reset\n");
            while (true) {}
            break;
        }
        case ID_PRT:
        {
            // The config of the green LED is locked during SafeIo::init.
            // Any write call will have no effect.

            Logger::debug(SAFETY, "Try to change write-protected config of green LED");
            ::bios::Io::PinConfiguration pinConfig;
            ::bios::Io::getConfiguration(::bios::Io::EVAL_LED_GREEN, pinConfig);
            pinConfig.pinCfg &= ~::bios::Io::ALT2;
            ::bios::Io::setConfiguration(::bios::Io::EVAL_LED_GREEN, pinConfig);

            // Check if the change was successful.
            ::bios::Io::getConfiguration(::bios::Io::EVAL_LED_GREEN, pinConfig);
            if (::bios::Io::ALT2 == (pinConfig.pinCfg & ::bios::Io::ALT2))
            {
                Logger::debug(SAFETY, "Write-protection worked, config of green LED not changed");
            }
            else
            {
                Logger::debug(SAFETY, "Write-protection failed, config of green LED changed");
            }
            break;
        }
        case ID_DST:
        {
            // SAFETY_TEST is monitored by SafeIo.
            // Disabling the pin will enter the safe state.
            Logger::debug(SAFETY, "Disable SAFETY_TEST pin");
            ::bios::Io::PinConfiguration pinConfig;
            ::bios::Io::getConfiguration(::bios::Io::SAFETY_TEST, pinConfig);
            pinConfig.dir = ::bios::Io::_DISABLED;
            ::bios::Io::setConfiguration(::bios::Io::SAFETY_TEST, pinConfig);
            break;
        }
        case ID_EST:
        {
            // SAFETY_TEST is monitored by SafeIo.
            // Re-enabling the pin will leave the safe state.
            Logger::debug(SAFETY, "Configure SAFETY_TEST pin as input");
            ::bios::Io::PinConfiguration pinConfig;
            ::bios::Io::getConfiguration(::bios::Io::SAFETY_TEST, pinConfig);
            pinConfig.dir = ::bios::Io::_IN;
            ::bios::Io::setConfiguration(::bios::Io::SAFETY_TEST, pinConfig);
            break;
        }
        default:
        {
            break;
        }
    }
}

} // namespace safety
