// Copyright 2024 Accenture.

#include "console/ConsoleMenu.h"

#include "console/Console.h"

#include <bsp/SystemTime.h>
#include <commandInterpreterSimple/CommandLine.h>
#include <reset/softwareSystemReset.h>

#include <commonDebug.h>

namespace bios
{
CommandSpec const ConsoleMenu::fCiSpec[] = {
    {"bsp", "bsp console"},
    {"t", "read system time"},
    {"7", "link information"},
    {"r", "reset"},
    {"d", "dreset"},
    {0, 0},
};

extern "C"
{
extern char const* getBinFileName(void);
}

ConsoleMenu::ConsoleMenu(Console& console)
: fCommandInterpreter(
    fCiSpec[0],
    CommandInterpreter::CommandFunc::create<ConsoleMenu, &ConsoleMenu::commandFunction>(*this))
{
    console.add(&fCommandInterpreter);
}

CommandReturnCode ConsoleMenu::commandFunction(CommandLine& commandLine)
{
    if (commandLine.getMatchingCommandIndex(0U, fCiSpec[0]) != 0U) // Main Menu
    {
        return CMD_NOCMD;
    }

    switch (static_cast<int32_t>(commandLine.getMatchingCommandIndex(1U, fCiSpec[0])))
    {
        case 1: // System Time
        {
            (void)printf("current time: %u", static_cast<unsigned int>(getSystemTimeUs()));
        }
        break;
        case 2: // Link Date
        {
            (void)printf(" %s\n", getBinFileName());
        }
        break;
        case 3:
        {
            softwareSystemReset();
            break;
        }
        case 4:
        {
            softwareDestructiveReset();
            break;
        }
        default:
        {
            return CMD_NOCMD;
        }
    }

    return CMD_OK;
}

} /* namespace bios */
