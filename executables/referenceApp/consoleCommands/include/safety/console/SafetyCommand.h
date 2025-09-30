// Copyright 2025 Accenture.

#pragma once

#include <util/command/GroupCommand.h>

namespace safety
{

class SafetyCommand : public ::util::command::GroupCommand
{
public:
    SafetyCommand() = default;

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;
};

} // namespace safety
