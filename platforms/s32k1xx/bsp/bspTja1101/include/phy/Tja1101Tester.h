// Copyright 2025 Accenture.

#pragma once

#include "util/command/CommandContext.h"
#include "util/command/GroupCommand.h"

namespace enetphy
{
class Tja1101;

class Tja1101Tester : public ::util::command::GroupCommand
{
public:
    Tja1101Tester(Tja1101& ethernetPhyTja1101);

protected:
    enum Commands
    {
        CMD_ALL,
        CMD_GET,
        CMD_STRT,
        CMD_STOP
    };

    DECLARE_COMMAND_GROUP_GET_INFO
    void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;

private:
    Tja1101& _ethernetPhyTja1101;
};

} // namespace enetphy
