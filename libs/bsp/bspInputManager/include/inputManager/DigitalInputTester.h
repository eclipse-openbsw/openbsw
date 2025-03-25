// Copyright 2024 Accenture.

#pragma once

#include "util/command/GroupCommand.h"

namespace bios
{
class DigitalInputTester : public ::util::command::GroupCommand
{
public:
    DigitalInputTester() {}

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;
};

} /* namespace bios */

