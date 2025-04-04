// Copyright 2024 Accenture.

#pragma once

#include "util/format/AttributedString.h"

#include <cstdint>

namespace util
{
namespace format
{
/**
 * Helper class that keeps track of VT100 compliant text attributes. Attributes are not set
 * immediately but function objects will be returned on each call which then can be evaluated lazily
 * by calling the operator() on a writer class. The formatter itself needs to be alive during this
 * evaluation because it keeps track of the current attributes to only write the necessary VT100
 * codes to change the text style as needed. Together with a ::util::format::StringWriter
 * it can be used as follows:
 *
 * \code{.cpp}
 * ::util::format::StringWriter writer(stream);
 * ::util::format::Vt100AttributedStringFormatter vt100;
 *
 * writer.printf("Mixed ")
 *       .apply(vt100.attr(::util::format::COLOR_GREEN, ::util::format::BOLD))
 *       .printf("%s", "string")
 *       .apply(vt100.attr(::util::format::COLOR_YELLOW))
 *       .printf(" attributes")
 *       .apply(vt100.reset());
 * \endcode
 *
 * The code above prints the following text parts:
 * - "Mixed "
 * - VT100-Codes for switching text color to green and formatting to bold
 * - "string"
 * - VT100-Codes for switching off bold formatting and changing text color to yellow
 * - " attributes"
 * - VT100-Codes for reseting text color to default
 */
class Vt100AttributedStringFormatter
{
public:
    class ApplyAttributes;
    class WriteAttributedString;

    /**
     * Create function object that resets all string attributes to default.
     * \return Function object for lazy evaluation
     */
    ApplyAttributes reset();
    /**
     * Create function object that sets the string attributes to the desired values.
     * \param attributes object holding the desired attributes
     * \return Function object for lazy evaluation
     */
    ApplyAttributes attr(StringAttributes const& attributes);
    /**
     * Create function object that sets the string attributes to the desired values.
     * \param foregroundColor desired foreground color. Use DEFAULT_COLOR for no change
     * \param format bitmask consisting of enum ::util::format::Format values that define
     * the desired formatting
     * \param backgroundColor desired background color. Use DEFAULT_COLOR for no change
     * \return Function object for lazy evaluation
     */
    ApplyAttributes
    attr(Color foregroundColor, uint8_t fmt = 0U, Color backgroundColor = Color::DEFAULT_COLOR);
    /**
     * Create function object that writes the desired attributed string to the stream. The
     * attributes are reset to its previous values after writing.
     * \param attributes string to write
     * \return Function object for lazy evaluation
     */
    WriteAttributedString write(AttributedString const& string);
    /**
     * Create function object that writes the desired attributed string to the stream. The
     * attributes are reset to its previous values after writing.
     * \param pure C-style zero-terminated string to write
     * \param attributes desired attributes for string output
     * \return Function object for lazy evaluation
     */
    WriteAttributedString write(char const* str, StringAttributes const& attributes);
    /**
     * Create function object that writes the desired attributed string to the stream. The
     * attributes are reset to its previous values after writing.
     * \param pure C-style zero-terminated string to write
     * \param foregroundColor desired foreground color
     * \param format desired formatting flags
     * \param backgroundColor desired background color for output
     * \return Function object for lazy evaluation
     */
    WriteAttributedString write(
        char const* str,
        Color foregroundColor,
        uint8_t fmt           = 0U,
        Color backgroundColor = Color::DEFAULT_COLOR);

public:
    /**
     * Function object that applies desired string attributes to the Vt100 formatter on call.
     */
    class ApplyAttributes
    {
    public:
        ApplyAttributes(
            Vt100AttributedStringFormatter& formatter, StringAttributes const& attributes)
        : _formatter(formatter), _attributes(attributes)
        {}

        template<class Writer>
        void apply(Writer& writer) const
        {
            _formatter.writeAttributes(writer, _attributes);
        }

    private:
        Vt100AttributedStringFormatter& _formatter;
        StringAttributes _attributes;
    };

    /**
     * Function object that writes a desired attributes string using the Vt100 formatter on call.
     */
    class WriteAttributedString
    {
    public:
        WriteAttributedString(
            Vt100AttributedStringFormatter& formatter, AttributedString const& string)
        : _formatter(formatter), _string(string)
        {}

        template<class Writer>
        void apply(Writer& writer) const
        {
            _formatter.write(writer, _string);
        }

    private:
        Vt100AttributedStringFormatter& _formatter;
        AttributedString _string;
    };

private:
    friend class ApplyAttributes;
    friend class WriteAttributedString;

    template<class Writer>
    void write(Writer& writer, AttributedString const& string)
    {
        StringAttributes const prevAttributes = _attributes;
        writeAttributes(writer, string.getAttributes());
        (void)writer.write(string.getString());
        writeAttributes(writer, prevAttributes);
    }

    template<class Writer>
    void writeAttributes(Writer& writer, StringAttributes const& attributes)
    {
        if (attributes != _attributes)
        {
            if (_attributes.isAttributed())
            {
                writeAttributes(writer, &_resetFormatCode, 1U);
            }
            _attributes = attributes;
            uint8_t codeBuffer[2U + NUMBER_OF_FORMATS];
            writeAttributes(writer, codeBuffer, getFormatCodes(attributes, codeBuffer));
        }
    }

    template<class Writer>
    static void
    writeAttributes(Writer& writer, uint8_t const* const formatCodes, uint8_t const length)
    {
        if (length > 0U)
        {
            (void)writer.write("\x1b[");
            for (uint8_t i = 0U; i < length; ++i)
            {
                (void)writer.write((i != 0U) ? ";" : nullptr).printf("%d", formatCodes[i]);
            }
            (void)writer.write("m");
        }
    }

    static uint8_t getFormatCodes(StringAttributes const& attributes, uint8_t* formatCodeBuffer);

    static uint8_t const _resetFormatCode;

    StringAttributes _attributes;
};

} // namespace format
} // namespace util
