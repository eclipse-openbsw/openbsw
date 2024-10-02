// Copyright 2024 Accenture.

#ifndef GUARD_7DE692F5_A5F9_46A4_A7D9_536D7B62322C
#define GUARD_7DE692F5_A5F9_46A4_A7D9_536D7B62322C

#include <bsp/commandInterpreter/ICommandInterpreterSimple.h>

#include <estd/uncopyable.h>
#include <platform/estdint.h>

namespace bios
{
class CommandModule;
class CommandLine;

class CommandInterpreter : public ICommandInterpreterSimple
{
    UNCOPYABLE(CommandInterpreter);

public:
    CommandInterpreter(CommandSpec const& cs, CommandFunc func);

    CommandReturnCode processCommand(CommandLine& commandLine) override;
    void printHelp(char const* specific = nullptr) override;
    char const* errorCodeToString(CommandReturnCode code) const;

private:
    CommandInterpreter();
    CommandFunc fCommandFunc;
    CommandSpec const& fCommands;
};

} // namespace bios

#endif
