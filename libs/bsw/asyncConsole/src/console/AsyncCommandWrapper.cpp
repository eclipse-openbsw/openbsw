// Copyright 2024 Accenture.

#include "console/AsyncCommandWrapper.h"

#include "console/AsyncConsole.h"

#include <async/Async.h>

namespace console
{
AsyncCommandWrapper::AsyncCommandWrapper(
    ::util::command::ICommand& command, ::async::ContextType context)
: fCommand(command), fArguments(), fpSharedOutputStream(0), fContext(context)
{
    AsyncConsole::addCommand(*this);
}

char const* AsyncCommandWrapper::getId() const { return fCommand.getId(); }

void AsyncCommandWrapper::getHelp(::util::command::ICommand::IHelpCallback& callback) const
{
    fCommand.getHelp(callback);
}

::util::command::ICommand::ExecuteResult AsyncCommandWrapper::execute(
    ::util::string::ConstString const& arguments,
    ::util::stream::ISharedOutputStream* sharedOutputStream)
{
    fArguments           = arguments;
    fpSharedOutputStream = sharedOutputStream;

    ::async::execute(fContext, *this);

    return ::util::command::ICommand::ExecuteResult(::util::command::ICommand::Result::OK);
}

void AsyncCommandWrapper::execute()
{
    ::util::command::ICommand::ExecuteResult const result
        = fCommand.execute(fArguments, fpSharedOutputStream);
    AsyncConsole::commandExecuted(result);
}

} /* namespace console */
