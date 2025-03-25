// Copyright 2024 Accenture.

#pragma once

#include "util/command/GroupCommand.h"

namespace bios
{

class AnalogTester : public ::util::command::GroupCommand
{
public:
    AnalogTester() {}

protected:
    DECLARE_COMMAND_GROUP_GET_INFO
    virtual void executeCommand(class ::util::command::CommandContext& context, uint8_t idx);
};

} // namespace bios

