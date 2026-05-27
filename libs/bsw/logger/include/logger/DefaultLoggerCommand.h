/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
