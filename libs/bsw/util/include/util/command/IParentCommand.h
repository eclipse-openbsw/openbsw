// Copyright 2024 Accenture.

#ifndef GUARD_BBB72C9B_D5EF_4909_BFFD_D5A5F5C2E087
#define GUARD_BBB72C9B_D5EF_4909_BFFD_D5A5F5C2E087

#include "util/command/ICommand.h"

namespace util
{
namespace command
{
class IParentCommand
{
public:
    IParentCommand() = default;

    IParentCommand(IParentCommand const&)            = delete;
    IParentCommand& operator=(IParentCommand const&) = delete;

    virtual void addCommand(ICommand& command) = 0;
    virtual void clearCommands()               = 0;
};
} // namespace command
} // namespace util

#endif /* GUARD_BBB72C9B_D5EF_4909_BFFD_D5A5F5C2E087 */
