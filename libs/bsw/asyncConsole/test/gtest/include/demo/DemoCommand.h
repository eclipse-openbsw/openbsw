#ifndef GUARD_988E20A7_FCA4_4055_A82E_826FCE85E1C6
#define GUARD_988E20A7_FCA4_4055_A82E_826FCE85E1C6

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

#endif /*GUARD_988E20A7_FCA4_4055_A82E_826FCE85E1C6*/
