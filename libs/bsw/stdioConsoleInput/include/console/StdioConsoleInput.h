// Copyright 2024 Accenture.

#pragma once

#include <etl/delegate.h>
#include <etl/string.h>
#include <util/stream/SharedOutputStream.h>
#include <util/stream/StdoutStream.h>
#include <util/stream/TaggedSharedOutputStream.h>

namespace console
{
/**
 * StdioConsoleInput is a class that enables users to define the response to stdin.
 */
class StdioConsoleInput
{
public:
    using OnLineProcessed = ::etl::delegate<void(void)>;

    /**
     * OnLineReceived is a user callback type.
     */
    using OnLineReceived = ::etl::delegate<void(
        ::util::stream::ISharedOutputStream& outputStream,
        ::etl::istring const& line,
        OnLineProcessed const& onLineProcessed)>;

public:
    StdioConsoleInput(StdioConsoleInput const&)     = delete;
    StdioConsoleInput& operator=(StdioConsoleInput) = delete;

    /**
     * Initialize the StdioConsoleInput object with a prefix and suffix that will be used to
     * decorate console output. (see util::stream::TaggedOutputHelper).
     */
    explicit StdioConsoleInput(char const* prefix, char const* suffix);

    /**
     * Initialize the StdioConsoleInput object with the provided callback function.
     * This callback function will later be called to react to the input.
     * \param onLineReceived user provided callback. By default, the reaction to any command will be
     * to clear the line buffer and print the prompt.
     */
    void init(OnLineReceived const& onLineReceived);

    /**
     * Shuts down the StdioConsoleInput object by resetting the callback function to a default
     * value.
     */
    void shutdown();

    /**
     * This function runs the StdioConsoleInput, processing user input from the console. If the
     * input is not suspended and a line is successfully read from the console, it processes the
     * input. If the input is empty, it prints a prompt to the console.
     * This function has to be called if it is known that stdin has an input that requires handling.
     */
    void run();

private:
    void onLineReceived(
        ::util::stream::ISharedOutputStream& outputStream,
        ::etl::istring const&,
        OnLineProcessed const& callback);
    void onLineProcessed();

private:
    ::util::stream::StdoutStream _stdoutStream;
    ::util::stream::SharedOutputStream _sharedOutputStream;
    ::util::stream::TaggedSharedOutputStream _taggedSharedOutputStream;

    ::etl::string<128> _line;
    OnLineReceived _onLineReceived;
    bool _isSuspended;
};

} /* namespace console */
