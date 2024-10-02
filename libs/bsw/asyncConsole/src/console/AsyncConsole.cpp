// Copyright 2024 Accenture.

#include "console/AsyncConsole.h"

#include "console/SyncCommandWrapper.h"
#include "logger/ConsoleLogger.h"

#include <util/command/HelpCommand.h>
#include <util/command/ParentCommand.h>
#include <util/format/SharedStringWriter.h>

#include <estd/string.h>

namespace
{
::util::command::ParentCommand& getParentCommand()
{
    static ::util::command::ParentCommand parentCommand("root", "root");
    return parentCommand;
}
} // namespace

namespace console
{
using ::util::logger::CONSOLE;
using ::util::logger::Logger;

AsyncConsole::AsyncConsole()
: ::estd::singleton<AsyncConsole>(*this), fpOutputStream(nullptr), fOnLineProcessed()
{
    static ::util::command::HelpCommand helpCommand(getParentCommand());
    static ::console::SyncCommandWrapper syncCommandWrapper(helpCommand);
}

void AsyncConsole::addCommand(::util::command::ICommand& command)
{
    getParentCommand().addCommand(command);
}

void AsyncConsole::commandExecuted(::util::command::ICommand::ExecuteResult result)
{
    AsyncConsole::instance().terminate(result);
}

void AsyncConsole::onLineReceived(
    ::util::stream::ISharedOutputStream& outputStream,
    ::estd::string const& line,
    OnLineProcessed const& onLineProcessed)
{
    auto const cmd = ::util::string::ConstString(line.c_str(), line.length());
    Logger::info(CONSOLE, "Received console command \"%.*s\"", cmd.length(), cmd.data());

    fOnLineProcessed = onLineProcessed;
    fpOutputStream   = &outputStream;

    ::util::command::ICommand::ExecuteResult const result
        = getParentCommand().execute(cmd, &outputStream);

    if (result.getResult() == ::util::command::ICommand::Result::OK)
    { // in this case, we terminate since commandExecuted is called
        return;
    }

    Logger::info(CONSOLE, "Console command failed");

    ::util::format::SharedStringWriter sharedStringWriter(outputStream);
    sharedStringWriter.printf("error\n");

    onLineProcessed();
}

void AsyncConsole::terminate(::util::command::ICommand::ExecuteResult result)
{
    if (fpOutputStream != nullptr)
    {
        ::util::format::StringWriter stringWriter(fpOutputStream->startOutput(nullptr));

        switch (result.getResult())
        {
            case ::util::command::ICommand::Result::OK:
            {
                Logger::info(CONSOLE, "Console command succeeded");
                stringWriter.printf("ok\n");
                break;
            }
            default:
            {
                Logger::info(CONSOLE, "Console command failed");
                stringWriter.printf("error\n");
                break;
            }
        }
        fpOutputStream = nullptr;
    }

    OnLineProcessed onLineProcessed = fOnLineProcessed;
    fOnLineProcessed                = OnLineProcessed();
    onLineProcessed();
}

} /* namespace console */
