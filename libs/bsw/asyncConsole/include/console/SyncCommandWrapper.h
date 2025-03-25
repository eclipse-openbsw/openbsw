// Copyright 2024 Accenture.

#pragma once

#include <util/command/ICommand.h>

namespace console
{
/**
 * This class allows a console command to be executed synchronously
 * in the same task context the console input is fetched. It wraps
 * the command and registers it at the AsyncConsole.
 *
 * \ingroup asyncConsole
 */
class SyncCommandWrapper : public ::util::command::ICommand
{
public:
    /**
     * SyncCommandWrapper
     * \param command Console command to wrap.
     */
    SyncCommandWrapper(::util::command::ICommand& command);

    /**
     * \see ::util::command::ICommand::getId
     */
    char const* getId() const override;

    /**
     * \see ::util::command::ICommand::getHelp
     */
    void getHelp(::util::command::ICommand::IHelpCallback& callback) const override;

    /**
     * \see ::util::command::ICommand::execute
     */
    ::util::command::ICommand::ExecuteResult execute(
        ::util::string::ConstString const& arguments,
        ::util::stream::ISharedOutputStream* sharedOutputStream) override;

private:
    ::util::command::ICommand& fCommand;
};

} /* namespace console */

