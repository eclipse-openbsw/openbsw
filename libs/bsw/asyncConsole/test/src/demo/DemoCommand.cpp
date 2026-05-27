/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "demo/DemoCommand.h"

#include <cstdio>

namespace demo
{

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(DemoCommand, "demo", "Demo Commands")
COMMAND_GROUP_COMMAND(1U, "hello", "Print hello")
DEFINE_COMMAND_GROUP_GET_INFO_END;

void DemoCommand::executeCommand(util::command::CommandContext& /* context */, uint8_t const idx)
{
    switch (idx)
    {
        case 1:
        {
            printf("Hello World");
            break;
        }
        default:
        {
            break;
        }
    }
}

} // namespace demo
