// Copyright 2024 Accenture.

#ifndef GUARD_358D8605_3018_46E9_ADAD_8B40199288A1
#define GUARD_358D8605_3018_46E9_ADAD_8B40199288A1

#include <async/Async.h>
#include <util/command/ICommand.h>

namespace console
{
/**
 * This class allows a console command to be executed asynchronously
 * in a task defined by an associated context. It wraps the command
 * and registers it at the AsyncConsole.
 *
 * \ingroup asyncConsole
 */
class AsyncCommandWrapper
: public ::util::command::ICommand
, private ::async::RunnableType
{
public:
    /**
     * AsyncCommandWrapper
     * \param command Console command to wrap.
     * \param context Context of the desired execution task.
     */
    AsyncCommandWrapper(::util::command::ICommand& command, ::async::ContextType context);

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
    void execute() override;

    ::util::command::ICommand& fCommand;

    ::util::string::ConstString fArguments;
    ::util::stream::ISharedOutputStream* fpSharedOutputStream;

    ::async::ContextType const fContext;
};

} /* namespace console */

#endif /* GUARD_358D8605_3018_46E9_ADAD_8B40199288A1 */
