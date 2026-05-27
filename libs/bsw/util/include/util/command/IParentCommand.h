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
