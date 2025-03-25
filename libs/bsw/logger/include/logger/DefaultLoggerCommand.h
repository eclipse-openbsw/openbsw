// Copyright 2024 Accenture.

#pragma once

#include "logger/IComponentConfig.h"

#include <util/command/ParentCommand.h>
#include <util/command/SimpleCommand.h>

namespace logger
{
class DefaultLoggerCommand
{
public:
    explicit DefaultLoggerCommand(IComponentConfig& componentConfig);

    ::util::command::ParentCommand& root() { return _root; }

private:
    void levelCommand(::util::command::CommandContext& context);

    IComponentConfig& _componentConfig;
    ::util::command::SimpleCommand _levelCommand;
    ::util::command::ParentCommand _root;
};

} /* namespace logger */

