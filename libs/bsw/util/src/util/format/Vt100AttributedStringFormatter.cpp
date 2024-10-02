// Copyright 2024 Accenture.

#include "util/format/Vt100AttributedStringFormatter.h"

namespace util
{
namespace format
{
uint8_t const Vt100AttributedStringFormatter::_resetFormatCode = 0U;
static uint8_t const foregroundColorCodes[]
    = {39U, 30U, 31U, 32U, 33U, 34U, 35U, 36U, 37U, 90U, 91U, 92U, 93U, 94U, 95U, 96U, 97U};
static uint8_t const backgroundColorCodes[]
    = {49U, 40U, 41U, 42U, 43U, 44U, 45U, 46U, 47U, 100U, 101U, 102U, 103U, 104U, 105U, 106U, 107U};
static uint8_t const formattingCodes[] = {1U, 2U, 4U, 5U, 7U, 8U};

Vt100AttributedStringFormatter::ApplyAttributes Vt100AttributedStringFormatter::reset()
{
    return ApplyAttributes(*this, StringAttributes());
}

Vt100AttributedStringFormatter::ApplyAttributes
Vt100AttributedStringFormatter::attr(StringAttributes const& attributes)
{
    return ApplyAttributes(*this, attributes);
}

Vt100AttributedStringFormatter::ApplyAttributes Vt100AttributedStringFormatter::attr(
    Color const foregroundColor, uint8_t const fmt, Color const backgroundColor)
{
    return ApplyAttributes(*this, StringAttributes(foregroundColor, fmt, backgroundColor));
}

Vt100AttributedStringFormatter::WriteAttributedString
Vt100AttributedStringFormatter::write(AttributedString const& string)
{
    return WriteAttributedString(*this, string);
}

Vt100AttributedStringFormatter::WriteAttributedString
Vt100AttributedStringFormatter::write(char const* const str, StringAttributes const& attributes)
{
    return WriteAttributedString(*this, AttributedString(str, attributes));
}

Vt100AttributedStringFormatter::WriteAttributedString Vt100AttributedStringFormatter::write(
    char const* const str,
    Color const foregroundColor,
    uint8_t const fmt,
    Color const backgroundColor)
{
    return WriteAttributedString(
        *this, AttributedString(str, StringAttributes(foregroundColor, fmt, backgroundColor)));
}

uint8_t Vt100AttributedStringFormatter::getFormatCodes(
    StringAttributes const& attributes, uint8_t* const formatCodeBuffer)
{
    uint8_t* currentFormatCode = formatCodeBuffer;
    if (attributes.getForegroundColor() != Color::DEFAULT_COLOR)
    {
        *currentFormatCode
            = foregroundColorCodes[static_cast<uint8_t>(attributes.getForegroundColor())];
        currentFormatCode++;
    }
    if (attributes.getBackgroundColor() != Color::DEFAULT_COLOR)
    {
        *currentFormatCode
            = backgroundColorCodes[static_cast<uint8_t>(attributes.getBackgroundColor())];
        currentFormatCode++;
    }
    if (attributes.getFormat() != 0U)
    {
        for (uint8_t checkFormat = 0U; checkFormat < NUMBER_OF_FORMATS; ++checkFormat)
        {
            if ((attributes.getFormat() & (static_cast<uint8_t>(1U << checkFormat))) > 0U)
            {
                *currentFormatCode = formattingCodes[checkFormat];
                currentFormatCode++;
            }
        }
    }
    return static_cast<uint8_t>(currentFormatCode - formatCodeBuffer);
}

} // namespace format
} // namespace util
