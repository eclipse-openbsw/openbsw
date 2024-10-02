// Copyright 2024 Accenture.

#include "commandInterpreterSimple/CommandInterpreter.h"

#include "commandInterpreterSimple/CommandLine.h"

#include <bsp/charInputOutput/Bspio.h>

#include <cstring>

namespace bios
{
CommandInterpreter::CommandInterpreter(CommandSpec const& cs, CommandFunc const func)
: fCommandFunc(func), fCommands(cs)
{}

CommandReturnCode CommandInterpreter::processCommand(CommandLine& commandLine)
{
    if (commandLine.getNumberOfTokens() < 1U)
    {
        return CMD_ERROR;
    }
    if (0 == strncmp(commandLine.getToken(0U), "help?", 4U))
    {
        printHelp(commandLine.getToken(1U));
        return CMD_OK;
    }

    CommandReturnCode const result = fCommandFunc(commandLine);
    if (result == CMD_NOCMD)
    {
        if (getNext() != nullptr)
        {
            return getNext()->processCommand(commandLine);
        }
        else
        {
            return CMD_ERROR;
        }
    }
    else
    {
        return result;
    }
}

char const* CommandInterpreter::errorCodeToString(CommandReturnCode const code) const
{
    static char const errorCode[static_cast<size_t>(CMD_NUM_CODES) + 1][6] = {
        "ok",
        "error",
        "nocmd",
        "",
    };
    return errorCode[code];
}

void CommandInterpreter::printHelp(char const* specific)
{
    CommandSpec const* c = &fCommands;
    if ((specific == nullptr) || ((specific != nullptr) && (strcmp(specific, c->fName) == 0)))
    {
        (void)debug_printf("\r\n***************\r\n");
        while (c->fName != nullptr)
        {
            (void)debug_printf("%s - %s\n", c->fName, c->fHelpText);
            ++c;
        }
    }
    ICommandInterpreterSimple::printHelp(specific);
}

} // namespace bios
