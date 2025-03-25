// Copyright 2024 Accenture.

#pragma once

#include "util/format/IPrintfArgumentReader.h"
#include "util/stream/IOutputStream.h"
#include "util/string/ConstString.h"

#include <cstdarg>
#include <cstdint>

namespace util
{
namespace format
{
/**
 * This class provides a simple way for formatting text to an arbitrary output stream.
 * Whenever text output is needed a StringWriter can be instantiated that wraps a
 * ::util::stream::IOutputStream and then provides a Printf-like way for
 * formatting both constant strings and data.
 *
 * All output methods of the writer returns a reference to the writer object itself which
 * allows an arbitrary number of function calls within a single statement.
 *
 * For a detailed description of supported format strings see class
 * ::util::format::PrintfFormatter.
 */
class StringWriter
{
public:
    /**
     * constructor.
     * \param stream the output stream to write into
     */
    explicit StringWriter(::util::stream::IOutputStream& strm) : _stream(strm) {}

    StringWriter(StringWriter const&)            = delete;
    StringWriter& operator=(StringWriter const&) = delete;

    /**
     * Writes a end-of-line character (\\n) to the stream.
     * \return reference to this writer
     */
    StringWriter& endl();
    /**
     * Writes a single character to the stream.
     * \param c character to write to the stream
     * \return reference to this writer
     */
    StringWriter& write(char c);
    /**
     * Writes a C-style (zero-terminated) string to the stream.
     * \param string zero-terminated string to write to the stream
     * \return reference to this writer
     */
    StringWriter& write(char const* str);
    /**
     * Writes an array of characters of given length to the stream.
     * \param chars characters to write
     * \param length number of characters to write
     * \return reference to this writer
     */
    StringWriter& write(char const* chars, size_t length);
    /**
     * Writes a constant string object (consisting of character array and length) to the stream.
     * \param string constant string to write
     * \return reference to this writer
     */
    StringWriter& write(::util::string::ConstString const& str);
    /**
     * Writes formatted data to the stream.
     * \param formatString the Printf-like format string (see
     * ::util::format::PrintfFormatter for a detailed description)
     * \param ... all arguments needed corresponding to the format string.
     * \return reference to this writer
     */
    StringWriter& printf(char const* formatString, ...);
    /**
     * Writes formatted data from a variable argument list to the stream.
     * \param formatString the Printf-like format string (see
     * ::util::format::PrintfFormatter for a detailed description)
     * \param va_list the arguments encapsulated in a va_list corresponding to the format string.
     * \return reference to this writer
     */
    StringWriter& vprintf(char const* formatString, va_list ap);
    /**
     * Writes formatted data from a variable argument list to the stream.
     * \param formatString the Printf-like format string (see
     * ::util::format::PrintfFormatter for a detailed description)
     * \param argReader all arguments encapsulated in an interface corresponding to the format
     * string.
     * \return reference to this writer
     */
    StringWriter& vprintf(char const* formatString, IPrintfArgumentReader& argReader);

    /**
     * Calls the given function object with the writer instance.
     * \param manip reference to function object for which operator() will be called with the
     * writer instance as its only parameter
     * \return reference to this writer
     */
    template<class T>
    StringWriter& apply(T const& manip)
    {
        manip.apply(*this);
        return *this;
    }

private:
    ::util::stream::IOutputStream& _stream;
};

} // namespace format
} // namespace util
