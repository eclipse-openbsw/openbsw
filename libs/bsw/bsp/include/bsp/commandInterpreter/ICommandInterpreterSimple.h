// Copyright 2024 Accenture.

#ifndef GUARD_BCEFE50D_DF7B_4E67_B24D_A6639AB5B41F
#define GUARD_BCEFE50D_DF7B_4E67_B24D_A6639AB5B41F

#include "CommandSpec.h"
#include "ReturnCode.h"

#include <estd/functional.h>
#include <estd/uncopyable.h>

namespace bios
{
class CommandLine;

class ICommandInterpreterSimple
{ // lint --e{970}    CommandInterpreter uses C strings, so usage of char is OK
    UNCOPYABLE(ICommandInterpreterSimple);

public:
    using CommandFunc = ::estd::function<CommandReturnCode(CommandLine&)>;

    ICommandInterpreterSimple() : next(nullptr) {}

    virtual CommandReturnCode processCommand(CommandLine& commandLine)
    {
        if (next != nullptr)
        {
            return (next->processCommand(commandLine));
        }
        return CMD_NOCMD;
    }

    virtual void printHelp(char const* specific = nullptr)
    {
        if (next != nullptr)
        {
            next->printHelp(specific);
        }
    }

    void add(ICommandInterpreterSimple* const client)
    {
        if ((next != client) && (client != nullptr))
        {
            if (next != nullptr)
            {
                next->add(client);
            }
            else
            {
                next = client;
            }
        }
    }

protected:
    ICommandInterpreterSimple* getNext() { return next; }

private:
    ICommandInterpreterSimple* next;
};

} /* namespace bios */

#endif /* GUARD_BCEFE50D_DF7B_4E67_B24D_A6639AB5B41F */
