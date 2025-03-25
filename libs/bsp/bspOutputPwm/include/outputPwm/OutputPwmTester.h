// Copyright 2024 Accenture.

#pragma once

#include "util/command/GroupCommand.h"

namespace bios
{

class OutputPwmTester : public ::util::command::GroupCommand
{
public:
    OutputPwmTester();

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(class ::util::command::CommandContext& context, uint8_t idx) override;
};

} // namespace bios

