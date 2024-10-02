// Copyright 2024 Accenture.

#ifndef GUARD_97386D2C_0BF8_41A1_A273_20933DAD856F
#define GUARD_97386D2C_0BF8_41A1_A273_20933DAD856F

#include "util/command/GroupCommand.h"

namespace bios
{
class DigitalInputTester : public ::util::command::GroupCommand
{
public:
    DigitalInputTester() {}

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;
};

} /* namespace bios */

#endif /* GUARD_97386D2C_0BF8_41A1_A273_20933DAD856F */
