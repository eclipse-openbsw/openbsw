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

#include <can/canframes/CANFrame.h>
#include <systems/ICanSystem.h>
#include <util/command/CommandContext.h>
#include <util/command/GroupCommand.h>
#include <util/format/SharedStringWriter.h>

namespace can
{
class CanCommand : public ::util::command::GroupCommand
{
public:
    CanCommand(::can::ICanSystem& system);

protected:
    enum Commands
    {
        CMD_INFO,
        CMD_SEND
    };

    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;

private:
    void send(::util::command::CommandContext& context, ::util::format::SharedStringWriter& writer);

    ::can::ICanSystem& _canSystem;
    CANFrame _canFrame;
};

} // namespace can
