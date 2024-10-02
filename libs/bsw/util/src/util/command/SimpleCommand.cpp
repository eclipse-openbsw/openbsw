// Copyright 2024 Accenture.

#include "util/command/SimpleCommand.h"

#include "util/command/CommandContext.h"

namespace util
{
namespace command
{
SimpleCommand::SimpleCommand(
    char const* const id, char const* const description, ExecuteFunction const executeFunc)
: _id(id), _description(description), _execute(executeFunc)
{}

char const* SimpleCommand::getDescription() const { return _description; }

char const* SimpleCommand::getId() const { return _id; }

ICommand::ExecuteResult SimpleCommand::execute(
    ::util::string::ConstString const& arguments,
    ::util::stream::ISharedOutputStream* const sharedOutputStream)
{
    if (sharedOutputStream != nullptr)
    {
        CommandContext context(arguments, sharedOutputStream);
        _execute(context);
        return ExecuteResult(context.getResult(), context.getSuffix(), this);
    }

    return ExecuteResult(Result::OK, arguments, this);
}

void SimpleCommand::getHelp(IHelpCallback& callback) const
{
    callback.startCommand(_id, _description, true);
}

} /* namespace command */
} /* namespace util */
