// Copyright 2024 Accenture.

#include "util/format/AttributedString.h"

namespace util
{
namespace format
{
AttributedString::AttributedString(PlainAttributedString const& attributedString)
: AttributedString(attributedString._string, attributedString._attributes)
{}

AttributedString::AttributedString(char const* const string, StringAttributes const& attributes)
: _string(string), _attributes(attributes)
{}

// explicit
StringAttributes::StringAttributes(
    Color const foregroundColor, uint8_t const fmt, Color const backgroundColor)
{
    _attributes._foregroundColor = foregroundColor;
    _attributes._format          = fmt;
    _attributes._backgroundColor = backgroundColor;
}

StringAttributes::StringAttributes(PlainStringAttributes const& attributes)
: _attributes(attributes)
{}

bool StringAttributes::operator==(StringAttributes const& other) const
{
    return (_attributes._foregroundColor == other._attributes._foregroundColor)
           && (_attributes._format == other._attributes._format)
           && (_attributes._backgroundColor == other._attributes._backgroundColor);
}

bool StringAttributes::operator!=(StringAttributes const& other) const
{
    return !operator==(other);
}

bool StringAttributes::isAttributed() const
{
    return (_attributes._foregroundColor != Color::DEFAULT_COLOR)
           || (_attributes._backgroundColor != Color::DEFAULT_COLOR) || (_attributes._format != 0U);
}

} // namespace format
} // namespace util
