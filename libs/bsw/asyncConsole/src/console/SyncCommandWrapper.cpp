/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "console/SyncCommandWrapper.h"

#include "console/AsyncConsole.h"

namespace console
{
SyncCommandWrapper::SyncCommandWrapper(::util::command::ICommand& command) : fCommand(command)
{
    AsyncConsole::addCommand(*this);
}

char const* SyncCommandWrapper::getId() const { return fCommand.getId(); }

void SyncCommandWrapper::getHelp(::util::command::ICommand::IHelpCallback& callback) const
{
    fCommand.getHelp(callback);
}

::util::command::ICommand::ExecuteResult SyncCommandWrapper::execute(
    ::util::string::ConstString const& arguments,
    ::util::stream::ISharedOutputStream* sharedOutputStream)
{
    ::util::command::ICommand::ExecuteResult const result
        = fCommand.execute(arguments, sharedOutputStream);
    AsyncConsole::commandExecuted(result);
    return ::util::command::ICommand::ExecuteResult(::util::command::ICommand::Result::OK);
}

} /* namespace console */
