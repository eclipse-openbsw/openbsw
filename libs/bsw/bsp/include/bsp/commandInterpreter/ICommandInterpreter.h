// Copyright 2024 Accenture.

#ifndef GUARD_EA8CB8A2_19E6_44ED_957D_762E86A1FA59
#define GUARD_EA8CB8A2_19E6_44ED_957D_762E86A1FA59

#include "CommandSpec.h"
#include "ReturnCode.h"

#include <estd/forward_list.h>
#include <estd/functional.h>
#include <estd/uncopyable.h>

namespace bios
{
class CommandInterpreter;

class CommandLine;

class ICommandInterpreter : public ::estd::forward_list_node<ICommandInterpreter>
{ // lint --e{970}    CommandInterpreter uses C strings, so usage of char is OK
    UNCOPYABLE(ICommandInterpreter);

public:
    using CommandFunc = ::estd::function<CommandReturnCode(CommandLine&)>;

    virtual CommandReturnCode processCommand(CommandLine& commandLine)  = 0;
    virtual void add(CommandInterpreter& child)                         = 0;
    virtual char const* getName() const                                 = 0;
    virtual char const* getHelpText() const                             = 0;
    virtual char const* errorCodeToString(CommandReturnCode code) const = 0;
    virtual void printHelp()                                            = 0;
};

} // namespace bios

#endif /* GUARD_EA8CB8A2_19E6_44ED_957D_762E86A1FA59 */
