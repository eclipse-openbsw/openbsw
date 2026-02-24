// Copyright 2024 Accenture.

#include "console/StdioConsoleInput.h"

#include <util/stream/BspStubs.h>

#include <platform/estdint.h>

namespace
{
constexpr char BACK_SPACE      = 0x08U;
constexpr char BACK_SPACE_7F   = 0x7FU;
constexpr char LINE_FEED       = 0x0AU;
constexpr char CARRIAGE_RETURN = 0x0DU;
constexpr char ESCAPE          = 0x1BU;

constexpr char MIN_VALID_CHR = 0x20U; // space
constexpr char MAX_VALID_CHR = 0x7EU; // tilde

/*
 * Reads a single character from standard input and appends it to the given string.
 *
 * \param line the string to which the character will be appended
 * \return true if the character read is a special character that indicates the end of input, false
 * otherwise.
 *
 * Special characters:
 * - ESCAPE: Clears the line and returns true
 * - BACK_SPACE or BACK_SPACE_7F: Removes the last character from the line if it is not empty
 * - CARRIAGE_RETURN or LINE_FEED: Indicates the end of input and returns true
 *
 * Valid characters are in the range [MIN_VALID_CHR, MAX_VALID_CHR]
 *
 * If the character read is less than or equal to 0, returns false
 */
bool getline(::etl::istring& line)
{
    char const chr = static_cast<char>(getByteFromStdin());

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
            putByteToStdout(BACK_SPACE);
            putByteToStdout(' ');
            putByteToStdout(BACK_SPACE);
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
: _stdoutStream()
, _sharedOutputStream(_stdoutStream)
, _taggedSharedOutputStream(_sharedOutputStream, prefix, suffix)
, _line()
, _onLineReceived(
      OnLineReceived::create<StdioConsoleInput, &StdioConsoleInput::onLineReceived>(*this))
, _isSuspended(false)
{}

void StdioConsoleInput::init(OnLineReceived const& onLineReceived)
{
    _onLineReceived = onLineReceived;
}

void StdioConsoleInput::shutdown()
{
    _onLineReceived
        = OnLineReceived::create<StdioConsoleInput, &StdioConsoleInput::onLineReceived>(*this);
}

void StdioConsoleInput::run()
{
    if (_isSuspended || !getline(_line))
    {
        return;
    }

    if (!_line.empty())
    {
        printNewline();
        _isSuspended = true;
        _onLineReceived(
            _taggedSharedOutputStream,
            _line,
            OnLineProcessed::create<StdioConsoleInput, &StdioConsoleInput::onLineProcessed>(*this));
    }
    else
    {
        printPrompt();
    }
}

void StdioConsoleInput::onLineReceived(
    ::util::stream::ISharedOutputStream& /*outputStream*/,
    ::etl::istring const& /*line*/,
    OnLineProcessed const& callback)
{
    callback();
}

void StdioConsoleInput::onLineProcessed()
{
    _line.clear();
    _isSuspended = false;
    printPrompt();
}

} /* namespace console */
