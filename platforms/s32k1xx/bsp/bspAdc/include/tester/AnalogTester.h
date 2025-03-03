// Copyright 2024 Accenture.

#ifndef GUARD_430869B5_2B89_4E1D_A90B_3735211DBBD0
#define GUARD_430869B5_2B89_4E1D_A90B_3735211DBBD0

#include "util/command/GroupCommand.h"

namespace bios
{

class AnalogTester : public ::util::command::GroupCommand
{
public:
    AnalogTester() {}

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(class ::util::command::CommandContext& context, uint8_t idx) override;
};

} // namespace bios

#endif /* GUARD_430869B5_2B89_4E1D_A90B_3735211DBBD0 */
