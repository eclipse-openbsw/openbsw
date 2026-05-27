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

#include "util/command/CommandContext.h"
#include "util/command/ICommand.h"

#include <etl/delegate.h>

namespace util
{
namespace command
{
class SimpleCommand : public ICommand
{
public:
    using ExecuteFunction = ::etl::delegate<void(CommandContext& context)>;

    SimpleCommand(char const* id, char const* description, ExecuteFunction executeFunc);

    char const* getDescription() const;

    char const* getId() const override;
    ExecuteResult execute(
        ::util::string::ConstString const& arguments,
        ::util::stream::ISharedOutputStream* sharedOutputStream) override;
    void getHelp(IHelpCallback& callback) const override;

private:
    char const* _id;
    char const* _description;
    ExecuteFunction _execute;
};

} /* namespace command */
} /* namespace util */
