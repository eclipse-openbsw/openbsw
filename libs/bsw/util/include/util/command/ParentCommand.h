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
#include "util/command/IParentCommand.h"

#include <etl/intrusive_forward_list.h>

namespace util
{
namespace command
{
class ParentCommand
: public ICommand
, public IParentCommand
{
public:
    ParentCommand(char const* id, char const* description);

    char const* getDescription() const;
    ::etl::intrusive_forward_list<ICommand, ::etl::forward_link<0>> const& getCommands() const;

    void addCommand(ICommand& command) override;
    void clearCommands() override;

    char const* getId() const override;
    ExecuteResult execute(
        ::util::string::ConstString const& arguments,
        ::util::stream::ISharedOutputStream* sharedOutputStream) override;
    void getHelp(IHelpCallback& callback) const override;

private:
    // workaround for large non virtual thunk
    void addCommand_local(ICommand& cmd);

    ICommand* lookupCommand(::util::string::ConstString const& id);

    char const* _id;
    char const* _description;
    ::etl::intrusive_forward_list<ICommand, ::etl::forward_link<0>> _commands;
};

} // namespace command
} // namespace util
