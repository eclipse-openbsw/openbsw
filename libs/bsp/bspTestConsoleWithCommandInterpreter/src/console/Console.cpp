// Copyright 2024 Accenture.

#include "console/Console.h"

#include <bsp/commandInterpreter/ICommandInterpreterSimple.h>
#include <charInputOutput/charIo.h>
#include <commandInterpreterSimple/CommandLine.h>

#include <commonDebug.h>

namespace bios
{
char Console::sfBuffer[CONSOLE_BUFFER_SIZE]  = {0};
char Console::sfBufferB[CONSOLE_BUFFER_SIZE] = {0x0};

Console::Console()
: ::estd::singleton<Console>(*this), fReadPosition(0U), fReadPrevPosition(0U), fConsoleClient(0)
{}

bool Console::execute()
{
    static bool showPrompt = true;

    if (showPrompt)
    {
        LOGSYNCHRON("\r\n> ");
        showPrompt = false;
    }
    if (gets(&(sfBuffer[0]), sizeof(sfBuffer)))
    {
        showPrompt = true;
        if ((sfBuffer[0] != 0) && fConsoleClient)
        {
            LOGSYNCHRON("\n");
            CommandLine command(sfBuffer);
            LOGSYNCHRON(
                "\r\n(%s)\r\n",
                fConsoleClient->errorCodeToString(fConsoleClient->processCommand(command)));
        }
    }
    return true;
}

bool Console::gets(char* const buffer, uint8_t const size)
{
    uint8_t const t = static_cast<uint8_t>(charIoGetc());

    if (t != 0U)
    {
        if (fReadPosition > (size - 1U))
        {
            fReadPosition = 0U;
        }

        if ((t != ESCAPE) && (t != CARRIAGE_RETURN) && (t != CURSOR_UP))
        {
            (void)charIoPutc(static_cast<int32_t>(t));
        }
        /* save content - cursor up */
        if (t == ESCAPE)
        {
            buffer[fReadPosition] = 0;
            fReadPosition         = 0U;
            return true;
        }
        else if (t == CURSOR_UP)
        {
            fReadPosition = fReadPrevPosition;
            for (uint8_t i = 0U; i < fReadPosition; i++)
            {
                buffer[i] = sfBufferB[i];
                (void)charIoPutc(static_cast<int32_t>(buffer[i]));
            }
            return false;
        }
        else if ((t == BACK_SPACE) || (t == BACK_SPACE_7F))
        {
            if (fReadPosition != 0U)
            {
                --fReadPosition;
            }
        }
        else if (t == CARRIAGE_RETURN)
        {
            fReadPrevPosition = fReadPosition;
            for (uint8_t i = 0U; i < fReadPrevPosition; i++)
            {
                sfBufferB[i] = buffer[i];
            }
            buffer[fReadPosition] = 0;
            fReadPosition         = 0U;
            return true;
        }
        else
        {
            buffer[fReadPosition] = static_cast<char>(t);
            ++fReadPosition;
        }
    }
    return false;
}

} /* namespace bios */
