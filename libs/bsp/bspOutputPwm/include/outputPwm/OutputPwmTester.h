// Copyright 2024 Accenture.

#ifndef GUARD_0E2647E3_8CE4_468E_BC74_748716A09DA5
#define GUARD_0E2647E3_8CE4_468E_BC74_748716A09DA5

#include "util/command/GroupCommand.h"

namespace bios
{

class OutputPwmTester : public ::util::command::GroupCommand
{
public:
    OutputPwmTester();

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(class ::util::command::CommandContext& context, uint8_t idx) override;
};

} // namespace bios

#endif /* GUARD_0E2647E3_8CE4_468E_BC74_748716A09DA5 */
