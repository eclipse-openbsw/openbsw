// Copyright 2024 Accenture.

#include "util/command/CommandContext.h"

namespace util
{
namespace command
{
using ::util::stream::IOutputStream;
using ::util::string::ConstString;

CommandContext::CommandContext(
    ConstString const& line, ISharedOutputStream* const sharedOutputStream)
: _nullStream()
, _sharedOutputStream(sharedOutputStream)
, _activeStream(nullptr)
, _start(line.data())
, _current(_start)
, _end(_start + line.length())
, _tokenStart(_start)
, _result(ICommand::Result::OK)
{
    (void)ignoreWhitespace();
}

bool CommandContext::hasToken() const { return isValid() && (_current != _end); }

ConstString CommandContext::scanToken()
{
    char const* const start = _current;
    if (isValid())
    {
        _tokenStart = _current;
        while ((_current != _end) && (!isWhitespace(*_current)))
        {
            ++_current;
        }
    }
    if (check(_current != start, ICommand::Result::BAD_TOKEN))
    {
        char const* const end = _current;
        (void)ignoreWhitespace();
        return ConstString(start, static_cast<size_t>(end - start));
    }

    return ConstString();
}

ConstString CommandContext::scanIdentifierToken()
{
    char const* const start = _current;
    if (isValid())
    {
        _tokenStart = _current;
        while ((_current != _end) && isIdentifierChar(*_current, _current == _tokenStart))
        {
            ++_current;
        }
    }
    char const* const end = _current;
    if (check((_current != start) && ignoreWhitespace(), ICommand::Result::BAD_TOKEN))
    {
        return ConstString(start, static_cast<size_t>(end - start));
    }

    return ConstString();
}

::etl::span<uint8_t> CommandContext::scanByteBufferToken(::etl::span<uint8_t> const& buf)
{
    char const* const start = _current;
    uint32_t pos            = 0U;
    if (isValid())
    {
        _tokenStart = _current;

        while ((_current != _end) && ((_current + 1) != _end))
        {
            int32_t const hiNibble = getDigit(*_current, 16U);
            if ((hiNibble >= 0) && check(pos < buf.size(), ICommand::Result::BAD_VALUE))
            {
                ++_current;
                int32_t const loNibble = getDigit(*_current, 16U);
                if (check(loNibble >= 0, ICommand::Result::BAD_TOKEN))
                {
                    buf[pos] = static_cast<uint8_t>(static_cast<uint8_t>(hiNibble) << 4U)
                               | static_cast<uint8_t>(loNibble);
                    ++pos;
                    ++_current;
                }
            }
            else
            {
                break;
            }
        }
    }
    if (check((_current != start) && ignoreWhitespace(), ICommand::Result::BAD_TOKEN))
    {
        return buf.first(pos);
    }

    return ::etl::span<uint8_t>();
}

bool CommandContext::checkEol()
{
    return check(_current == _end, ICommand::Result::UNEXPECTED_TOKEN);
}

bool CommandContext::check(bool const condition, ICommand::Result const result)
{
    if (isValid() && (!condition))
    {
        _result = result;
    }
    return isValid();
}

ICommand::Result CommandContext::getResult() const { return _result; }

ConstString CommandContext::getSuffix() const
{
    char const* const start = (_result == ICommand::Result::BAD_VALUE) ? _tokenStart : _current;
    return ConstString(start, static_cast<size_t>(_end - start));
}

IOutputStream& CommandContext::startOutput(IContinuousUser* const /* user */)
{
    if (_sharedOutputStream != nullptr)
    {
        _activeStream = &_sharedOutputStream->startOutput(nullptr);
        return *_activeStream;
    }

    return _nullStream;
}

void CommandContext::endOutput(IContinuousUser* const user)
{
    if (_sharedOutputStream != nullptr)
    {
        if ((_activeStream != nullptr) && (user != nullptr))
        {
            user->endContinuousOutput(*_activeStream);
        }
        _activeStream = nullptr;
        _sharedOutputStream->endOutput(nullptr);
    }
}

void CommandContext::releaseContinuousUser(IContinuousUser& /* user */) {}

bool CommandContext::isValid() const { return _result == ICommand::Result::OK; }

bool CommandContext::ignoreWhitespace()
{
    char const* const start = _current;
    while ((_current != _end) && isWhitespace(*_current))
    {
        ++_current;
    }
    return (_current != start) || (_current == _end);
}

// static
bool CommandContext::isWhitespace(char const c)
{
    switch (c)
    {
        case '\r':
        case '\n':
        case '\t':
        case ' ':
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}

// static
bool CommandContext::isIdentifierChar(char const c, bool const firstChar)
{
    if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_'))
    {
        return true;
    }
    if (!firstChar)
    {
        return getDigit(c, 10U) >= 0;
    }

    return false;
}

// static
int32_t CommandContext::getDigit(char const c, uint32_t const base)
{
    if ((c >= '0') && (c <= '7'))
    {
        return static_cast<int32_t>(c - '0');
    }
    if (base < 10U)
    {
        return -1;
    }

    if ((c == '8') || (c == '9'))
    {
        return static_cast<int32_t>(c - '0');
    }
    if (base != 16U)
    {
        return -1;
    }

    if ((c >= 'a') && (c <= 'f'))
    {
        return static_cast<int32_t>(c - ('a' - 10));
    }
    if ((c >= 'A') && (c <= 'F'))
    {
        return static_cast<int32_t>(c - ('A' - 10));
    }

    return -1;
}

} // namespace command
} // namespace util
