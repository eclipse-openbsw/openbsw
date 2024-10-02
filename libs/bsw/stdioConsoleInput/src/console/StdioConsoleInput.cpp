// Copyright 2024 Accenture.

#include "console/StdioConsoleInput.h"

#include <util/stream/BspStubs.h>

#include <platform/estdint.h>

namespace
{
static char const BACK_SPACE      = 0x08U;
static char const BACK_SPACE_7F   = 0x7FU;
static char const LINE_FEED       = 0x0AU;
static char const CARRIAGE_RETURN = 0x0DU;
static char const ESCAPE          = 0x1BU;

static char const MIN_VALID_CHR = 0x20U; // space
static char const MAX_VALID_CHR = 0x7EU; // tilde

bool getline(::estd::string& line)
{
    char const chr = getByteFromStdin();
    if (chr <= 0)
    {
        return false;
    }

    if (line.full())
    {
        line.clear();
    }

    if ((MIN_VALID_CHR <= chr) && (chr <= MAX_VALID_CHR))
    {
        line.append(1, chr);
        putByteToStdout(chr);
        return false;
    }

    if (ESCAPE == chr)
    {
        line.clear();
        return true;
    }

    if ((BACK_SPACE == chr) || (BACK_SPACE_7F == chr))
    {
        if (!line.empty())
        {
            putByteToStdout(chr);
            line.resize(line.length() - 1);
        }
        return false;
    }

    if ((CARRIAGE_RETURN == chr) || (LINE_FEED == chr))
    {
        return true;
    }

    return false;
}

void printNewline()
{
    putByteToStdout('\r');
    putByteToStdout('\n');
}

void printPrompt()
{
    printNewline();
    putByteToStdout('>');
    putByteToStdout(' ');
}

} // namespace

namespace console
{
StdioConsoleInput::StdioConsoleInput(char const* const prefix, char const* const suffix)
: fStdoutStream()
, fSharedOutputStream(fStdoutStream)
, fTaggedSharedOutputStream(fSharedOutputStream, prefix, suffix)
, fLine()
, fOnLineReceived(
      OnLineReceived::create<StdioConsoleInput, &StdioConsoleInput::onLineReceived>(*this))
, fIsSuspended(false)
{}

void StdioConsoleInput::init(OnLineReceived const& onLineReceived)
{
    fOnLineReceived = onLineReceived;
}

void StdioConsoleInput::shutdown()
{
    fOnLineReceived
        = OnLineReceived::create<StdioConsoleInput, &StdioConsoleInput::onLineReceived>(*this);
}

void StdioConsoleInput::run()
{
    if (fIsSuspended)
    {
        return;
    }

    if (!getline(fLine))
    {
        return;
    }

    if (fLine.empty())
    {
        printPrompt();
        return;
    }
    else
    {
        printNewline();
    }

    fIsSuspended = true;
    fOnLineReceived(
        fTaggedSharedOutputStream,
        fLine,
        OnLineProcessed::create<StdioConsoleInput, &StdioConsoleInput::onLineProcessed>(*this));
}

void StdioConsoleInput::onLineReceived(
    ::util::stream::ISharedOutputStream& /*outputStream*/,
    ::estd::string const& /*l*/,
    OnLineProcessed const& callback)
{
    callback();
}

void StdioConsoleInput::onLineProcessed()
{
    fLine.clear();
    fIsSuspended = false;
    printPrompt();
}

} /* namespace console */
