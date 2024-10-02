// Copyright 2024 Accenture.

#ifndef GUARD_2CE17B68_8C70_46F8_9936_75BFCAC70A27
#define GUARD_2CE17B68_8C70_46F8_9936_75BFCAC70A27

#include "util/command/GroupCommand.h"

namespace bios
{
class OutputTester : public ::util::command::GroupCommand
{
public:
    OutputTester() {}

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(class ::util::command::CommandContext& context, uint8_t idx) override;
};

} /* namespace bios */

#endif /* GUARD_2CE17B68_8C70_46F8_9936_75BFCAC70A27 */
