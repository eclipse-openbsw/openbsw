// Copyright 2024 Accenture.

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
