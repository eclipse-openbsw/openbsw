#pragma once

// BEGIN DEMOCOMMAND
#include "util/command/GroupCommand.h"

namespace demo
{

class DemoCommand : public util::command::GroupCommand
{
public:
    DemoCommand() = default;

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(util::command::CommandContext& context, uint8_t idx) override;
};

} // namespace demo

// END DEMOCOMMAND

