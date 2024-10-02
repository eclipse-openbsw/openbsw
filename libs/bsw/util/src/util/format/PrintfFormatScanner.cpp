// Copyright 2024 Accenture.

#include "util/format/PrintfFormatScanner.h"

namespace util
{
namespace format
{
PrintfFormatScanner::PrintfFormatScanner(char const* const formatString)
: _start(nullptr)
, _current((formatString != nullptr) ? formatString : "")
, _tokenType(TokenType::END)
, _paramInfo()
{
    nextToken();
}

void PrintfFormatScanner::nextToken()
{
    if (*_current == '%')
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
    _start = _current;
    if (*_current != '\0')
    {
        _tokenType = TokenType::STRING;
        _current += offset;
        while ((*_current != '\0') && (*_current != '%'))
        {
            ++_current;
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
    _start            = _current;
    _paramInfo._flags = 0U;
    ++_current;
    scanParamFlags();
    _paramInfo._width = scanWidthOrPrecision();
    if (*_current == '.')
    {
        ++_current;
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
    if (isDigit(*_current))
    {
        result = 0;
        do
        {
            result = (10 * result) + static_cast<int32_t>((*_current - '0'));
            ++_current;
        } while (isDigit(*_current));
    }
    else if (*_current == '*')
    {
        ++_current;
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
        switch (*_current)
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
        ++_current;
    }
}

uint8_t PrintfFormatScanner::scanParamLength()
{
    switch (*_current)
    {
        case 'h':
        {
            ++_current;
            return 1U;
        }
        case 'l':
        {
            ++_current;
            if (*_current == 'l')
            {
                ++_current;
            }
            return 3U;
        }
        case 'L':
        {
            ++_current;
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
    char const specifier = *_current;
    ++_current;
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
            --_current;
            scanString();
            break;
        }

        default:
        {
            ++_start;
            --_current;
            scanString(1U);
            break;
        }
    }
}
} // namespace format
} // namespace util
