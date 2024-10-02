// Copyright 2024 Accenture.

#ifndef GUARD_B76A30ED_678A_45F5_8854_EE6509A6D217
#define GUARD_B76A30ED_678A_45F5_8854_EE6509A6D217

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

#endif // GUARD_B76A30ED_678A_45F5_8854_EE6509A6D217
