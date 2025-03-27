// Copyright 2024 Accenture.

#pragma once

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
