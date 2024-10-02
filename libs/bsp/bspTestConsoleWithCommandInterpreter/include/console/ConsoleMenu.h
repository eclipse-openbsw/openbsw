// Copyright 2024 Accenture.

#ifndef GUARD_17FB1545_47BD_4BBE_8030_AAB7C59A36DB
#define GUARD_17FB1545_47BD_4BBE_8030_AAB7C59A36DB

#include <commandInterpreterSimple/CommandInterpreter.h>

namespace bios
{
class Console;

class ConsoleMenu
{
public:
    ConsoleMenu(Console& console);

    virtual bios::CommandReturnCode commandFunction(bios::CommandLine& commandLine);

protected:
    bios::CommandInterpreter fCommandInterpreter;

    static bios::CommandSpec const fCiSpec[];
};

} // namespace bios

#endif
