// Copyright 2024 Accenture.

#include "util/command/ParentCommand.h"

#include "util/command/CommandContext.h"
#include "util/stream/ISharedOutputStream.h"

#include <platform/config.h>

namespace util
{
namespace command
{
using ::util::stream::ISharedOutputStream;
using ::util::string::ConstString;

ParentCommand::ParentCommand(char const* const id, char const* const description)
: _id(id), _description(description), _commands()
{}

char const* ParentCommand::getDescription() const { return _description; }

::etl::intrusive_forward_list<ICommand, ::etl::forward_link<0>> const&
ParentCommand::getCommands() const
{
    return _commands;
}

ESR_NO_INLINE void ParentCommand::addCommand_local(ICommand& cmd)
{
    ::etl::intrusive_forward_list<ICommand, ::etl::forward_link<0>>::iterator prevIt;
    ::etl::intrusive_forward_list<ICommand, ::etl::forward_link<0>>::iterator it
        = _commands.begin();
    ConstString const id(cmd.getId());
    for (; (it != _commands.end()) && (id.compareIgnoreCase(ConstString(it->getId())) > 0); ++it)
    {
        prevIt = it;
    }
    if (it == _commands.begin())
    {
        _commands.push_front(cmd);
    }
    else
    {
        (void)_commands.insert_after(prevIt, cmd);
    }
}

// virtual
void ParentCommand::addCommand(ICommand& command) { addCommand_local(command); }

void ParentCommand::clearCommands() { _commands.clear(); }

// virtual
char const* ParentCommand::getId() const { return _id; }

// virtual
ICommand::ExecuteResult
ParentCommand::execute(ConstString const& arguments, ISharedOutputStream* const sharedOutputStream)
{
    CommandContext context(arguments, sharedOutputStream);
    ConstString const id = context.scanIdentifierToken();
    ICommand* const cmd  = lookupCommand(id);
    if (cmd != nullptr)
    {
        if (sharedOutputStream != nullptr)
        {
            return cmd->execute(context.getSuffix(), sharedOutputStream);
        }

        return ExecuteResult(Result::OK, context.getSuffix(), cmd);
    }

    return ExecuteResult(Result::NOT_RESPONSIBLE, arguments, nullptr);
}

// virtual
void ParentCommand::getHelp(IHelpCallback& callback) const
{
    callback.startCommand(_id, _description, false);
    for (auto const& _command : _commands)
    {
        _command.getHelp(callback);
    }
    callback.endCommand();
}

ICommand* ParentCommand::lookupCommand(ConstString const& id)
{
    for (auto& _command : _commands)
    {
        if (ConstString(_command.getId()).compareIgnoreCase(id) == 0)
        {
            return &_command;
        }
    }
    return nullptr;
}

} // namespace command
} // namespace util
