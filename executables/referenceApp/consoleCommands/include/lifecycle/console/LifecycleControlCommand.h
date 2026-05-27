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

#include <util/command/GroupCommand.h>

namespace lifecycle
{
class ILifecycleManager;
}

namespace lifecycle
{

class LifecycleControlCommand : public ::util::command::GroupCommand
{
public:
    LifecycleControlCommand(ILifecycleManager& lifecycleManager);

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;

private:
    ILifecycleManager& _lifecycleManager;
};

} // namespace lifecycle
