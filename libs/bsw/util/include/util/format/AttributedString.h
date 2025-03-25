// Copyright 2024 Accenture.

#pragma once

#include <cstdint>

namespace util
{
namespace format
{
/**
 * Colors that can be applied to attributed strings.
 */
enum class Color : uint8_t
{
    DEFAULT_COLOR = 0,
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    LIGHT_GRAY,
    DARK_GRAY,
    LIGHT_RED,
    LIGHT_GREEN,
    LIGHT_YELLOW,
    LIGHT_BLUE,
    LIGHT_MAGENTA,
    LIGHT_CYAN,
    WHITE,

    NUMBER_OF_COLORS
};

/**
 * Flags that can be combined to represent formatting switches for attributed strings.
 */
uint8_t const BOLD      = 0x01U;
uint8_t const DIM       = 0x02U;
uint8_t const UNDERLINE = 0x04U;
uint8_t const BLINK     = 0x08U;
uint8_t const REVERSE   = 0x10U;
uint8_t const HIDDEN    = 0x20U;

uint8_t const NUMBER_OF_FORMATS = 6U;

/**
 * A simple structure that can be used to stored string attributes constantly.
 */
struct PlainStringAttributes
{
    PlainStringAttributes() = default;

    constexpr PlainStringAttributes(Color const foregroundColor)
    : PlainStringAttributes(foregroundColor, 0U, Color::DEFAULT_COLOR)
    {}

    constexpr PlainStringAttributes(Color const foregroundColor, uint8_t const format)
    : PlainStringAttributes(foregroundColor, format, Color::DEFAULT_COLOR)
    {}

    constexpr PlainStringAttributes(
        Color const foregroundColor, uint8_t const format, Color const backgroundColor)
    : _foregroundColor(foregroundColor), _format(format), _backgroundColor(backgroundColor)
    {}

    Color _foregroundColor;
    uint8_t _format;
    Color _backgroundColor;
};

/**
 * Wrapper class for string attributes.
 */
class StringAttributes
{
public:
    /**
     * constructor.
     * \param foregroundColor desired foreground color
     * \param format bitmask of formatting flags (see enum ::util::format::Format)
     * \parrm backgroundColor desired background color
     */
    explicit StringAttributes(
        Color foregroundColor = Color::DEFAULT_COLOR,
        uint8_t fmt           = 0U,
        Color backgroundColor = Color::DEFAULT_COLOR);
    StringAttributes(PlainStringAttributes const& attributes);

    /**
     * Check whether a non-default value is set to any of the contained fields.
     * \return true if any non-default value is set
     */
    bool isAttributed() const;

    /**
     * Get a reference to the string attributes.
     * \return reference to the plain string attributes
     */
    PlainStringAttributes const& getAttributes() const { return _attributes; }

    /**
     * \return the desired foreground color
     */
    Color getForegroundColor() const { return _attributes._foregroundColor; }

    /**
     * \return bitmask holding the flags for the desired formatting
     */
    uint8_t getFormat() const { return _attributes._format; }

    /**
     * \return the desired background color
     */
    Color getBackgroundColor() const { return _attributes._backgroundColor; }

    bool operator==(StringAttributes const& other) const;
    bool operator!=(StringAttributes const& other) const;

private:
    PlainStringAttributes _attributes;
};

/**
 * A simple structure for storing a zero-terminated string together with string attributes.
 * Can be used to place both text and formatting in constant memory.
 */
struct PlainAttributedString
{
    char const* _string;
    PlainStringAttributes _attributes;
};

/**
 * helper class for holding both zero-terminated string and the string attributes to apply.
 */
class AttributedString
{
public:
    /**
     * constructor.
     * \param string zero-terminated string
     * \param attributes string attributes to apply
     */
    AttributedString(char const* string, StringAttributes const& attributes);
    AttributedString(PlainAttributedString const& attributedString);

    /**
     * \return the zero-terminated string
     */
    inline char const* getString() const { return _string; }

    /**
     * \return the string attributes to apply.
     */
    inline StringAttributes const& getAttributes() const { return _attributes; }

private:
    char const* _string;
    StringAttributes _attributes;
};

} // namespace format
} // namespace util

