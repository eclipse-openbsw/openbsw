// Copyright 2024 Accenture.

#pragma once

#include "util/command/CommandContext.h"
#include "util/command/ICommand.h"

#include <estd/functional.h>

namespace util
{
namespace command
{
class SimpleCommand : public ICommand
{
public:
    using ExecuteFunction = ::estd::function<void(CommandContext& context)>;

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

