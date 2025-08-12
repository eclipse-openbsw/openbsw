// Copyright 2024 Accenture.

#include "util/format/PrintfFormatScanner.h"

namespace util
{
namespace format
{
PrintfFormatScanner::PrintfFormatScanner(char const* const formatString)
: _start(nullptr)
, _currentPosition((formatString != nullptr) ? formatString : "")
, _tokenType(TokenType::END)
, _paramInfo()
{
    nextToken();
}

void PrintfFormatScanner::nextToken()
{
    if (*_currentPosition == '%')
    {
        scanParam();
    }
    else
    {
        scanString();
    }
}

void PrintfFormatScanner::scanString(uint32_t const offset)
{
    _start = _currentPosition;
    if (*_currentPosition != '\0')
    {
        _tokenType = TokenType::STRING;
        _currentPosition += offset;
        while ((*_currentPosition != '\0') && (*_currentPosition != '%'))
        {
            ++_currentPosition;
        }
    }
    else
    {
        _tokenType = TokenType::END;
    }
}

void PrintfFormatScanner::scanParam()
{
    _tokenType        = TokenType::PARAM;
    _start            = _currentPosition;
    _paramInfo._flags = 0U;
    ++_currentPosition;
    scanParamFlags();
    _paramInfo._width = scanWidthOrPrecision();
    if (*_currentPosition == '.')
    {
        ++_currentPosition;
        _paramInfo._precision = scanWidthOrPrecision();
    }
    else
    {
        _paramInfo._precision = ParamWidthOrPrecision::DEFAULT;
    }
    uint8_t const intPower = scanParamLength();
    scanParamFormatSpecifier(intPower);
}

int32_t PrintfFormatScanner::scanWidthOrPrecision()
{
    int32_t result;
    if (isDigit(*_currentPosition))
    {
        result = 0;
        do
        {
            result = (10 * result) + static_cast<int32_t>((*_currentPosition - '0'));
            ++_currentPosition;
        } while (isDigit(*_currentPosition));
    }
    else if (*_currentPosition == '*')
    {
        ++_currentPosition;
        result = ParamWidthOrPrecision::PARAM;
    }
    else
    {
        result = ParamWidthOrPrecision::DEFAULT;
    }
    return result;
}

void PrintfFormatScanner::scanParamFlags()
{
    while (true)
    {
        switch (*_currentPosition)
        {
            case '-':
            {
                _paramInfo._flags |= ParamFlags::FLAG_LEFT;
                break;
            }
            case '+':
            {
                _paramInfo._flags |= ParamFlags::FLAG_PLUS;
                break;
            }
            case ' ':
            {
                _paramInfo._flags |= ParamFlags::FLAG_SPACE;
                break;
            }
            case '#':
            {
                _paramInfo._flags |= ParamFlags::FLAG_ALT;
                break;
            }
            case '0':
            {
                _paramInfo._flags |= ParamFlags::FLAG_ZEROPAD;
                break;
            }
            default:
            {
                return;
            }
        }
        ++_currentPosition;
    }
}

uint8_t PrintfFormatScanner::scanParamLength()
{
    switch (*_currentPosition)
    {
        case 'h':
        {
            ++_currentPosition;
            return 1U;
        }
        case 'l':
        {
            ++_currentPosition;
            if (*_currentPosition == 'l')
            {
                ++_currentPosition;
            }
            return 3U;
        }
        case 'L':
        {
            ++_currentPosition;
            return 2U;
        }
        default:
        {
            return 2U;
        }
    }
}

void PrintfFormatScanner::scanParamFormatSpecifier(uint8_t const intPower)
{
    char const specifier = *_currentPosition;
    ++_currentPosition;
    switch (specifier)
    {
        case 'c':
        {
            setParamType(ParamType::CHAR, ParamDatatype::UINT8);
            break;
        }

        case 'd':
        case 'i':
        {
            setIntParamType(intPower, ParamDatatype::SINT8, 10U);
            break;
        }
        case 'u':
        {
            setIntParamType(intPower, ParamDatatype::UINT8, 10U);
            break;
        }

        case 'o':
        {
            setIntParamType(intPower, ParamDatatype::UINT8, 8U);
            break;
        }

        case 'n':
        {
            setParamType(ParamType::POS, ParamDatatype::SINT32PTR);
            break;
        }

        case 'X':
        {
            _paramInfo._flags |= ParamFlags::FLAG_UPPER;
            setIntParamType(intPower, ParamDatatype::UINT8, 16U);
            break;
        }
        case 'x':
        {
            setIntParamType(intPower, ParamDatatype::UINT8, 16U);
            break;
        }

        case 'p':
        {
            setParamType(ParamType::PTR, ParamDatatype::VOIDPTR, 16U);
            break;
        }

        case 's':
        {
            setParamType(ParamType::STRING, ParamDatatype::CHARPTR);
            break;
        }
        case 'S':
        {
            setParamType(ParamType::STRING, ParamDatatype::SIZEDCHARPTR);
            break;
        }

        case 0:
        {
            --_currentPosition;
            scanString();
            break;
        }

        default:
        {
            ++_start;
            --_currentPosition;
            scanString(1U);
            break;
        }
    }
}
} // namespace format
} // namespace util
