// Copyright 2024 Accenture.

#ifndef GUARD_D95DA684_BEC3_403B_B628_31E72CC87FDB
#define GUARD_D95DA684_BEC3_403B_B628_31E72CC87FDB

#include "util/command/ICommand.h"
#include "util/command/IParentCommand.h"

#include <estd/forward_list.h>

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
    ::estd::forward_list<ICommand> const& getCommands() const;

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
    ::estd::forward_list<ICommand> _commands;
};

} // namespace command
} // namespace util

#endif /* GUARD_D95DA684_BEC3_403B_B628_31E72CC87FDB */
