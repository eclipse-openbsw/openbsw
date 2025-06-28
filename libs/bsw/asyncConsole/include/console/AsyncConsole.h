// Copyright 2024 Accenture.

#pragma once

#include <etl/delegate.h>
#include <etl/singleton_base.h>
#include <etl/string.h>
#include <etl/uncopyable.h>
#include <util/command/ICommand.h>
#include <util/stream/ISharedOutputStream.h>

namespace console
{
/**
 * This class consumes a line, dispatches it to the registered
 * console commands and notifies its caller whenever the line is
 * processed by a console command. The registration of the console
 * commands is done by means of the enclosed command wrapper classes
 * (e.g. AsyncCommandWrapper).
 *
 * \ingroup asyncConsole
 */
class AsyncConsole
: public ::etl::singleton_base<AsyncConsole>
, private ::etl::uncopyable
{
public:
    using OnLineProcessed = ::etl::delegate<void(void)>;

    /**
     * AsyncConsole
     */
    AsyncConsole();

    /**
     * Forward line to the registered console commands.
     * The line is not copied in order to save resources.
     * As long as onLineProcessed is not called, the caller must not modify line.
     * \param outputStream The stream where to write output.
     * \param line The line to forward.
     * \param onLineProcessed Callback to inform the caller that the line is consumed.
     */
    void onLineReceived(
        ::util::stream::ISharedOutputStream& outputStream,
        ::etl::istring const& line,
        OnLineProcessed const& onLineProcessed);

private:
    friend class AsyncCommandWrapper;
    friend class SyncCommandWrapper;

    static void addCommand(::util::command::ICommand& command);
    static void commandExecuted(::util::command::ICommand::ExecuteResult result);

    void terminate(::util::command::ICommand::ExecuteResult result);

    ::util::stream::ISharedOutputStream* fpOutputStream;
    OnLineProcessed fOnLineProcessed;
};

} /* namespace console */
