#include "demo/DemoCommand.h"

#include <cstdio>

namespace demo
{

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(DemoCommand, "demo", "Demo Commands")
COMMAND_GROUP_COMMAND(1U, "hello", "Print hello")
DEFINE_COMMAND_GROUP_GET_INFO_END;

void DemoCommand::executeCommand(util::command::CommandContext& /* context */, uint8_t const idx)
{
    switch (idx)
    {
        case 1:
        {
            printf("Hello World");
            break;
        }
        default:
        {
            break;
        }
    }
}

} // namespace demo
