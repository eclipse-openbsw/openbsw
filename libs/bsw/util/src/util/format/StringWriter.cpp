// Copyright 2024 Accenture.

#include "util/format/StringWriter.h"

#include "util/format/PrintfArgumentReader.h"
#include "util/format/PrintfFormatter.h"

#include <estd/va_list_ref.h>

namespace util
{
namespace format
{
using ::util::string::ConstString;

StringWriter& StringWriter::endl()
{
    _stream.write(static_cast<uint8_t>('\n'));
    return *this;
}

StringWriter& StringWriter::write(char const c)
{
    _stream.write(static_cast<uint8_t>(c));
    return *this;
}

StringWriter& StringWriter::write(char const* const str)
{
    if (str != nullptr)
    {
        (void)(write(str, strlen(str)));
    }
    return *this;
}

StringWriter& StringWriter::write(char const* const chars, size_t const length)
{
    _stream.write(::estd::slice<uint8_t const>::from_pointer(
        reinterpret_cast<uint8_t const*>(chars), length));
    return *this;
}

StringWriter& StringWriter::write(ConstString const& str)
{
    return write(str.data(), str.length());
}

StringWriter& StringWriter::printf(char const* const formatString, ...)
{
    va_list ap;
    va_start(ap, formatString);
    PrintfArgumentReader argReader(ap);
    static_cast<void>(vprintf(formatString, argReader));
    va_end(ap);
    return *this;
}

StringWriter& StringWriter::vprintf(char const* const formatString, va_list ap)
{
    PrintfArgumentReader argReader(ap);
    return vprintf(formatString, argReader);
}

StringWriter&
StringWriter::vprintf(char const* const formatString, IPrintfArgumentReader& argReader)
{
    if (formatString != nullptr)
    {
        PrintfFormatter formatter(_stream);
        formatter.format(formatString, argReader);
    }
    return *this;
}

} // namespace format
} // namespace util
