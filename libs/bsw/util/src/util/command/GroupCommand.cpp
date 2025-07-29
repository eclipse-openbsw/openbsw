// Copyright 2024 Accenture.

#include "util/command/GroupCommand.h"

#include "util/command/CommandContext.h"

namespace util
{
namespace command
{
using ::util::stream::ISharedOutputStream;
using ::util::string::ConstString;

char const* GroupCommand::getDescription() const { return getInfo()->_description; }

char const* GroupCommand::getId() const { return getInfo()->_id; }

ICommand::ExecuteResult
GroupCommand::execute(ConstString const& arguments, ISharedOutputStream* const sharedOutputStream)
{
    CommandContext context(arguments, sharedOutputStream);
    ConstString const id                             = context.scanToken();
    GroupCommand::PlainCommandInfo const* const info = lookupCommand(id);
    if (info != nullptr)
    {
        if (sharedOutputStream != nullptr)
        {
            executeCommand(context, info->_idx);
            return ExecuteResult(context.getResult(), context.getSuffix(), this);
        }

        return ExecuteResult(Result::OK, arguments, this);
    }

    return ExecuteResult(Result::NOT_RESPONSIBLE, arguments, nullptr);
}

void GroupCommand::getHelp(IHelpCallback& callback) const
{
    callback.startCommand(getId(), getDescription(), false);
    for (GroupCommand::PlainCommandInfo const* info = getInfo() + 1; info->_id != nullptr; ++info)
    {
        callback.startCommand(info->_id, info->_description, true);
    }
    callback.endCommand();
}

GroupCommand::PlainCommandInfo const* GroupCommand::lookupCommand(ConstString const& id) const
{
    for (GroupCommand::PlainCommandInfo const* info = getInfo() + 1; info->_id != nullptr; ++info)
    {
        if (ConstString(info->_id).compareIgnoreCase(id) == 0)
        {
            return info;
        }
    }
    return nullptr;
}

} /* namespace command */
} /* namespace util */
