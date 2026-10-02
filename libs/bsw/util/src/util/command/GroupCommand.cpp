/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/command/GroupCommand.h"

#include "util/command/CommandContext.h"

namespace util
{
namespace command
{
using ::util::stream::ISharedOutputStream;
using ::util::string::ConstString;

char const* GroupCommand::getDescription() const { return getInfo()[0]._description; }

char const* GroupCommand::getId() const { return getInfo()[0]._id; }

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
    CommandInfoTable const infos = getInfo();
    for (size_t idx = 1U; (idx < infos.size()) && (infos[idx]._id != nullptr); ++idx)
    {
        callback.startCommand(infos[idx]._id, infos[idx]._description, true);
    }
    callback.endCommand();
}

GroupCommand::PlainCommandInfo const* GroupCommand::lookupCommand(ConstString const& id) const
{
    CommandInfoTable const infos = getInfo();
    for (size_t idx = 1U; (idx < infos.size()) && (infos[idx]._id != nullptr); ++idx)
    {
        if (ConstString(infos[idx]._id).compareIgnoreCase(id) == 0)
        {
            return &infos[idx];
        }
    }
    return nullptr;
}

} /* namespace command */
} /* namespace util */
