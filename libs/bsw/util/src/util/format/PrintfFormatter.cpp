// Copyright 2024 Accenture.

#include "util/format/PrintfFormatter.h"

#include "util/format/PrintfArgumentReader.h"
#include "util/format/PrintfFormatScanner.h"

namespace util
{
namespace format
{
using ::util::stream::IOutputStream;

PrintfFormatter::PrintfFormatter(IOutputStream& strm, bool const writeParam)
: _stream(strm), _writeParam(writeParam), _pos(0U)
{}

// NOLINTNEXTLINE(cert-dcl50-cpp): va_list usage only for printing functionalities.
void PrintfFormatter::format(char const* const formatString, ...)
{
    va_list ap;
    va_start(ap, formatString);
    format(formatString, ap);
    va_end(ap);
}

void PrintfFormatter::format(char const* const formatString, va_list ap)
{
    PrintfArgumentReader argReader(ap);
    format(formatString, argReader);
}

void PrintfFormatter::format(char const* const formatString, IPrintfArgumentReader& argReader)
{
    for (PrintfFormatScanner scanner(formatString); scanner.hasToken(); scanner.nextToken())
    {
        if (scanner.getTokenType() == TokenType::STRING)
        {
            formatText(
                scanner.getTokenStart(),
                static_cast<size_t>(scanner.getTokenEnd() - scanner.getTokenStart()));
        }
        else
        {
            if (scanner.needsWidthParam())
            {
                ParamVariant const* const widthParam
                    = argReader.readArgument(ParamDatatype::SINT32);
                scanner.setWidth(
                    (widthParam != nullptr) ? widthParam->_sint32Value
                                            : ParamWidthOrPrecision::DEFAULT);
            }
            if (scanner.needsPrecisionParam())
            {
                ParamVariant const* const precisionParam
                    = argReader.readArgument(ParamDatatype::SINT32);
                scanner.setPrecision(
                    (precisionParam != nullptr) ? precisionParam->_sint32Value
                                                : ParamWidthOrPrecision::DEFAULT);
            }
            ParamVariant const* const pArgument
                = argReader.readArgument(scanner.getParamInfo()._datatype);
            if (pArgument != nullptr)
            {
                formatParam(scanner.getParamInfo(), *pArgument);
            }
            else
            {
                formatText("<?>", 3U);
            }
        }
    }
}

void PrintfFormatter::formatText(char const* const text, size_t const length)
{
    putString(text, length);
}

void PrintfFormatter::formatParam(ParamInfo const& paramInfo, ParamVariant const& variant)
{
    switch (paramInfo._type)
    {
        case ParamType::CHAR:
        {
            formatStringParam(paramInfo, reinterpret_cast<char const*>(&variant._uint8Value), 1U);
            break;
        }
        case ParamType::STRING:
        {
            if (paramInfo._datatype == ParamDatatype::CHARPTR)
            {
                formatStringParam(paramInfo, variant._charPtrValue);
            }
            else if (variant._sizedCharPtrValue != nullptr)
            {
                formatStringParam(
                    paramInfo,
                    variant._sizedCharPtrValue->_data,
                    variant._sizedCharPtrValue->_length);
            }
            else
            {
                formatStringParam(paramInfo, nullptr);
            }
            break;
        }
        case ParamType::INT:
        case ParamType::PTR:
        {
            formatIntParam(paramInfo, variant);
            break;
        }
        case ParamType::POS:
        {
            if (_writeParam && (variant._sint32PtrValue != nullptr))
            {
                *variant._sint32PtrValue = static_cast<int32_t>(_pos);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void PrintfFormatter::formatStringParam(ParamInfo const& paramInfo, char const* str)
{
    if (str == nullptr)
    {
        str = "<NULL>";
    }
    formatStringParam(paramInfo, str, strlen(str));
}

void PrintfFormatter::formatStringParam(
    ParamInfo const& paramInfo, char const* const str, size_t length)
{
    if ((paramInfo._precision >= 0) && (length > static_cast<uint32_t>(paramInfo._precision)))
    {
        length = static_cast<uint32_t>(paramInfo._precision);
    }
    fillWidth(paramInfo, static_cast<int32_t>(length), true);
    putString(str, length);
    fillWidth(paramInfo, static_cast<int32_t>(length), false);
}

void PrintfFormatter::formatIntParam(ParamInfo const& paramInfo, ParamVariant const& value)
{
    char buf[22];
    char* const pBufferEnd         = &buf[sizeof(buf)];
    int8_t sign                    = 0;
    char const* const pBufferStart = formatIntDatatype(pBufferEnd, paramInfo, value, sign);
    char const* const pSign        = getIntSign(paramInfo, sign);
    size_t const signLength        = strlen(pSign);
    char const* const pPrefix      = getIntPrefix(paramInfo, sign);
    size_t const prefixLength      = strlen(pPrefix);
    ptrdiff_t digitCount           = pBufferEnd - pBufferStart;
    ptrdiff_t totalCount
        = digitCount + static_cast<ptrdiff_t>(signLength) + static_cast<ptrdiff_t>(prefixLength);
    ptrdiff_t precision;
    if (paramInfo._precision >= 0)
    {
        if ((paramInfo._precision == 0) && (sign == 0))
        {
            digitCount = 0;
            if (totalCount > 0)
            {
                --totalCount;
            }
        }
        precision = (paramInfo._precision > digitCount) ? (paramInfo._precision - digitCount) : 0;
    }
    else if (
        ((paramInfo._flags & ParamFlags::FLAG_ZEROPAD) != 0U)
        && ((paramInfo._flags & ParamFlags::FLAG_LEFT) == 0U))
    {
        precision = (paramInfo._width > totalCount) ? (paramInfo._width - totalCount) : 0;
    }
    else
    {
        precision = 0;
    }
    totalCount += precision;
    fillWidth(paramInfo, static_cast<int32_t>(totalCount), true);
    putString(pSign, signLength);
    putString(pPrefix, prefixLength);
    putChar('0', static_cast<uint32_t>(precision));
    putString(pBufferStart, static_cast<size_t>(digitCount));
    fillWidth(paramInfo, static_cast<int32_t>(totalCount), false);
}

// static
char* PrintfFormatter::formatIntDatatype(
    char* const pBufferEnd, ParamInfo const& paramInfo, ParamVariant const& value, int8_t& sign)
{
    char const* const pDigits = ((paramInfo._flags & ParamFlags::FLAG_UPPER) > 0U)
                                    ? "0123456789ABCDEF"
                                    : "0123456789abcdef";

    char* ret;

    switch (paramInfo._datatype)
    {
        case ParamDatatype::SINT16:
        {
            uint32_t const castValue = static_cast<uint32_t>(value._sint16Value);
            uint32_t const baseValue = static_cast<uint32_t>(paramInfo._base);
            ret = formatIntDigits<uint32_t>(pBufferEnd, pDigits, castValue, true, baseValue, sign);
            break;
        }
        case ParamDatatype::SINT32:
        {
            uint32_t const castValue = static_cast<uint32_t>(value._sint32Value);
            uint32_t const baseValue = static_cast<uint32_t>(paramInfo._base);
            ret = formatIntDigits<uint32_t>(pBufferEnd, pDigits, castValue, true, baseValue, sign);
            break;
        }
        case ParamDatatype::SINT64:
        {
            uint64_t const castValue = static_cast<uint64_t>(value._sint64Value);
            uint64_t const baseValue = static_cast<uint64_t>(paramInfo._base);
            ret = formatIntDigits<uint64_t>(pBufferEnd, pDigits, castValue, true, baseValue, sign);
            break;
        }
        case ParamDatatype::UINT16:
        {
            uint32_t const castValue = static_cast<uint32_t>(value._uint16Value);
            uint32_t const baseValue = static_cast<uint32_t>(paramInfo._base);
            ret = formatIntDigits<uint32_t>(pBufferEnd, pDigits, castValue, false, baseValue, sign);
            break;
        }
        case ParamDatatype::UINT32:
        {
            uint32_t const baseValue = static_cast<uint32_t>(paramInfo._base);
            ret                      = formatIntDigits<uint32_t>(
                pBufferEnd, pDigits, value._uint32Value, false, baseValue, sign);
            break;
        }
        case ParamDatatype::UINT64:
        {
            uint64_t const baseValue = static_cast<uint64_t>(paramInfo._base);
            ret                      = formatIntDigits<uint64_t>(
                pBufferEnd, pDigits, value._uint64Value, false, baseValue, sign);
            break;
        }
        case ParamDatatype::VOIDPTR:
        {
            if (sizeof(void const*) == 4)
            {
                uint32_t const baseValue = static_cast<uint32_t>(paramInfo._base);
                ret                      = formatIntDigits<uint32_t>(
                    pBufferEnd, pDigits, value._uint32Value, false, baseValue, sign);
            }
            else
            {
                uint64_t const baseValue = static_cast<uint64_t>(paramInfo._base);
                ret                      = formatIntDigits<uint64_t>(
                    pBufferEnd, pDigits, value._uint64Value, false, baseValue, sign);
            }
            break;
        }
        default:
        {
            ret = pBufferEnd;
            break;
        }
    }

    return ret;
}

void PrintfFormatter::fillWidth(ParamInfo const& paramInfo, int32_t const length, bool const left)
{
    if ((paramInfo._width >= 0) && (((paramInfo._flags & ParamFlags::FLAG_LEFT) != 0U) != left)
        && (paramInfo._width > length))
    {
        putChar(' ', static_cast<size_t>(paramInfo._width - length));
    }
}

} // namespace format
} // namespace util
