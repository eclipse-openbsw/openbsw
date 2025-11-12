// Copyright 2024 Accenture.

#pragma once

#include "util/format/IPrintfArgumentReader.h"
#include "util/format/Printf.h"
#include "util/stream/IOutputStream.h"

#include <etl/type_traits.h>

#include <cstdarg>
#include <cstring>

namespace util
{
namespace format
{
/**
 * Formatter that supports printf like formatting functionality on a stream object.
 *
 * \tparam  Stream  Type of character stream to format into. A stream class has to offer two
 * functions:
 *                  \code{.cpp}void appendChar(char)\endcode for appending a single character and
 *                  \code{.cpp}void appendChars(char const*, uint32_t)\endcode for appending a
 *                  given number of characters.
 *
 * \section printf_formatter_format_strings Format strings
 *
 * All formatting starts with a format string. The PrintfFormatter class supports only a subset of
 * the formats the Linux GCC supports. The main difference is the lack of floating point formats.
 *
 * The format string is a character string, beginning and ending in its initial shift state,
 * if any. The format string is composed of zero or more directives: ordinary characters (not %),
 * which are copied unchanged to the output stream; and conversion specifications, each of which
 * results in fetching zero or more subsequent arguments. Each conversion specification is
 * introduced by the character %, and ends with a conversion specifier. In between there may be
 * (in this order) zero or more flags, an optional minimum field width, an optional precision and
 * an optional length modifier. The arguments must correspond properly (after type promotion) with
 * the conversion specifier. The arguments are used in the order given, where each '*' and each
 * conversion specifier asks for the next argument (and it is an error if insufficiently many
 * arguments are given). The implementation does not include the style using '$', which comes from
 * the Single UNIX Specification.
 *
 * \subsection printf_formatter_flag_characters Flag characters
 *
 * The character % is followed by zero or more of the following flags:
 *
 * \par #
 *     The value should be converted to an "alternate form". For o conversions, the first character
 *     of the output string is made zero (by prefixing a 0 if it was not zero already). For x and X
 *     conversions, a result has the string "0x" (or "0X" for X conversions) prepended to it.
 *
 * \par 0
 *     The value should be zero padded. For d, i, o, u, x, X the converted value is padded on the
 *     left with zeros rather than blanks. If the 0 and - flags both appear, the 0 flag is ignored.
 *     If a precision is given the 0 flag is ignored. For other conversions, the behavior is
 *     undefined.
 *
 * \par -
 *     The converted value is to be left adjusted on the field boundary. (The default is right
 *     justification.) Except for n conversions, the converted value is padded on the right with
 *     blanks, rather than on the left with blanks or zeros. A - overrides a 0 if both are given.
 *
 * \par ' '
 *     (a space) A blank should be left before a positive number (or empty string) produced by a
 *     signed conversion.
 *
 * \par +
 *     A sign (+ or -) should always be placed before a number produced by a signed conversion. By
 *     default a sign is used only for negative numbers. A + overrides a space if both are used.
 *
 * \subsection printf_formatter_field_width The field width
 *
 * An optional decimal digit string (with nonzero first digit) specifying a minimum field width. If
 * the converted value has fewer characters than the field width, it will be padded with spaces on
 * the left (or right, if the left-adjustment flag has been given). Instead of a decimal digit
 * string one may write "*" to specify that the field width is given in the next argument which
 * must be of type int. A negative field width is taken as a '-' flag followed by a positive field
 * width. In no case does a nonexistent or small field width cause truncation of a field; if the
 * result of a conversion is wider than the field width, the field is expanded to contain the
 * conversion result.
 *
 * \subsection printf_formatter_precision The precision
 *
 * An optional precision, in the form of a period ('.') followed by an optional decimal digit
 * string. Instead of a decimal digit string one may write "*" to specify that the precision
 * is given in the next argument which must be of type int. If the precision is given as just
 * '.', or the precision is negative, the precision is taken to be zero. This gives the
 * minimum number of digits to appear for d, i, o, u, x, and X conversions or the maximum
 * number of characters to be printed from a string for s conversions.
 *
 * \subsection printf_formatter_length_modifier The length modifier
 *
 * Here, "integer conversion" stands for d, i, o, u, x, or X conversion. The base datatype for
 * integers is always 32 bit. 64 bit values have to be marked as long, 16 bit values as short.
 *
 * \par h
 *     A following integer conversion corresponds to a signed or unsigned 16 bit integer argument,
 *     or a following n conversion corresponds to a pointer to a 16 bit integer argument.
 *
 * \par l or ll
 *     (ell or ell-ell) A following integer conversion corresponds to a signed or unsigned 64 bit
 *     integer argument, or a following n conversion corresponds to a pointer to a 64 bit
 *     integer argument.
 *
 * \subsection printf_formatter_conversion_specified The conversion specifier
 *
 * A character that specifies the type of conversion to be applied. The conversion specifiers and
 * their meanings are:
 *
 * \par d, i
 *     The int argument is converted to signed decimal notation. The precision, if any, gives the
 *     minimum number of digits that must appear; if the converted value requires fewer digits,
 *     it is padded on the left with zeros. The default precision is 1. When 0 is printed with an
 *     explicit precision 0, the output is empty.
 *
 * \par o, u, x, X
 *     The unsigned int argument is converted to unsigned octal (o), unsigned decimal (u), or
 *     unsigned hexadecimal (x and X) notation. The letters abcdef are used for x conversions;
 *     the letters ABCDEF are used for X conversions. The precision, if any, gives the minimum
 *     number of digits that must appear; if the converted value requires fewer digits, it is
 *     padded on the left with zeros. The default precision is 1. When 0 is printed with an
 *     explicit precision 0, the output is empty.
 *
 * \par c
 *     the int argument is converted to an unsigned char, and the resulting character is written.
 *
 * \par s
 *     The const char * argument is expected to be a pointer to an array of character type (pointer
 *     to a string). Characters from the array are written up to (but not including) a terminating
 *     null byte ('\0'); if a precision is specified, no more than the number specified
 *     are written. If a precision is given, no null byte need be present; if the precision is not
 *     specified, or is greater than the size of the array, the array must contain a terminating
 *     null byte.

 * \par S
 *     This is an non-standard extension: The argument is expected to be a pointer to a
 *     ::util::string::PlainSizedString structure that holds a pointer to an array of
 *     character type (pointer to a string) and the length in number of characters. If a precision
 *     is specified, no more than the number specified are written.
 *
 * \par p
 *     The void * pointer argument is printed in hexadecimal (as if by %#x or %#llx).
 *
 * \par n
 *     The number of characters written so far is stored into the integer indicated by the int*
 *     pointer argument. No argument is converted.
 *
 * \par %
 *     A '%%' is written. No argument is converted. The complete conversion specification is '%%%'.
 */
class PrintfFormatter
{
public:
    /**
     * Constructor to initialize the formatter to work on a certain stream.
     *
     * \param stream The stream to put characters into
     * \param writeParam This controls the behaviour when the %n conversion is used.
     *                   - true: allows to write back the number of characters written so far to
     *                           the  specified int * pointer argument.
     *                   - false: All %n arguments are skipped without writing back the position.
     */
    explicit PrintfFormatter(::util::stream::IOutputStream& strm, bool writeParam = true);
    PrintfFormatter(PrintfFormatter const&)            = delete;
    PrintfFormatter& operator=(PrintfFormatter const&) = delete;

    /**
     * Writes the C string pointed by format to the encapsulated stream. If format includes format
     * specifiers (subsequences beginning with %), the additional arguments following format are
     * formatted and inserted in the resulting string replacing their respective specifiers.
     *
     * \param formatString C string that contains the text to be written.
     * \param ... Depending on the format string, the function may expect a sequence of additional
     * arguments, each containing a value to be used to replace a format specifier in the format
     * string (or a pointer to a storage location, for n). There should be at least as many of these
     * arguments as the number of values specified in the format specifiers. Additional arguments
     * are ignored by the function.
     */
    // 2012: We use the ... syntax to support printf-style formatting
    void format(char const* formatString, ...);
    /**
     * Writes the C string pointed by format to the encapsulated stream. If format includes format
     * specifiers (subsequences beginning with %), the additional arguments following format are
     * formatted and inserted in the resulting string replacing their respective specifiers.
     *
     * \param formatString C string that contains the text to be written.
     * \param ap A value identifying a variable arguments list initialized with va_start. va_list is
     * a special type defined in <cstdarg>.
     */
    void format(char const* formatString, va_list ap);
    /**
     * Writes the C string pointed by format to the encapsulated stream. If format includes format
     * specifiers (subsequences beginning with %), the additional arguments following format are
     * formatted and inserted in the resulting string replacing their respective specifiers. Use
     * this function if you want to provide argument list in your own way.
     *
     * \param formatString C string that contains the text to be written.
     * \param argReader Reference to an interface that successively returns the arguments to be put
     * out.
     */
    void format(char const* formatString, IPrintfArgumentReader& argReader);

    /**
     * Writes a C string to the encapsulated stream without scanning for any format specifier. This
     * function is also used from inside the format functions to output constant text to the stream.
     *
     * \param text C string that contains the raw text to be written to the stream.
     * \param length number of characters to write to the stream
     */
    void formatText(char const* text, size_t length);
    /**
     * Converts an argument value and formats it to the encapsulated stream. This function is also
     * used from inside the format functions to output a converted argument value to the stream.
     *
     * \param paramInfo A structure that contains the formatting to be applied to the value.
     * \param variant The argument containing the value to write.
     */
    void formatParam(ParamInfo const& paramInfo, ParamVariant const& variant);

private:
    void formatStringParam(ParamInfo const& paramInfo, char const* str);
    void formatStringParam(ParamInfo const& paramInfo, char const* str, size_t length);
    void formatIntParam(ParamInfo const& paramInfo, ParamVariant const& value);
    static char* formatIntDatatype(
        char* pBufferEnd, ParamInfo const& paramInfo, ParamVariant const& value, int8_t& sign);
    template<class T>
    static inline char* formatIntDigits(
        char* bufferEnd,
        char const* digits,
        T value,
        bool signedType,
        typename ::etl::make_unsigned<T>::type base,
        int8_t& sign);
    static inline char const* getIntSign(ParamInfo const& paramInfo, int8_t sign);
    static inline char const* getIntPrefix(ParamInfo const& paramInfo, int8_t sign);

    void fillWidth(ParamInfo const& paramInfo, int32_t length, bool left);

    inline void putString(char const* string, size_t length);
    inline void putChar(char c, size_t count);

    ::util::stream::IOutputStream& _stream;
    bool _writeParam;
    size_t _pos;
};

/*
 * Implementation
 */
template<class T>
inline char* PrintfFormatter::formatIntDigits(
    char* bufferEnd,
    char const* const digits,
    T value,
    bool const signedType,
    typename ::etl::make_unsigned<T>::type const base,
    int8_t& sign)
{
    if (value != 0U)
    {
        if (signedType)
        {
            if (static_cast<typename ::etl::make_signed<T>::type>(value) < 0)
            {
                value = -value;
                sign  = -1;
            }
            else
            {
                sign = +1;
            }
        }
        else
        {
            sign = +1;
        }

        while (value > 0U)
        {
            --bufferEnd;
            *bufferEnd = digits[value % base];
            value /= base;
        }
    }
    else
    {
        sign = 0;
        --bufferEnd;
        *bufferEnd = '0';
    }
    return bufferEnd;
}

// static
inline char const* PrintfFormatter::getIntSign(ParamInfo const& paramInfo, int8_t const sign)
{
    char const* pSign = "";
    if (paramInfo._base == 10U)
    {
        switch (paramInfo._datatype)
        {
            case ParamDatatype::SINT8:
            case ParamDatatype::SINT16:
            case ParamDatatype::SINT32:
            case ParamDatatype::SINT64:
            {
                if (sign < 0)
                {
                    pSign = "-";
                }
                else if ((paramInfo._flags & ParamFlags::FLAG_PLUS) > 0U)
                {
                    pSign = "+";
                }
                else if ((paramInfo._flags & ParamFlags::FLAG_SPACE) > 0U)
                {
                    pSign = " ";
                }
                else
                {
                    // do nothing
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return pSign;
}

// static
inline char const* PrintfFormatter::getIntPrefix(ParamInfo const& paramInfo, int8_t const sign)
{
    char const* pPrefix = "";
    switch (paramInfo._base)
    {
        case 8:
        {
            if ((sign > 0) && ((paramInfo._flags & ParamFlags::FLAG_ALT) != 0U))
            {
                pPrefix = "0";
            }
            break;
        }
        case 16:
        {
            if ((paramInfo._flags & ParamFlags::FLAG_ALT) != 0U)
            {
                pPrefix = ((paramInfo._flags & ParamFlags::FLAG_UPPER) > 0U) ? "0X" : "0x";
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return pPrefix;
}

inline void PrintfFormatter::putString(char const* const string, size_t const length)
{
    _pos += length;
    _stream.write_string_view(::etl::string_view{string, length});
}

inline void PrintfFormatter::putChar(char const c, size_t count)
{
    _pos += count;
    while (count != 0U)
    {
        _stream.write(static_cast<uint8_t>(c));
        --count;
    }
}

} // namespace format
} // namespace util
