// Copyright 2024 Accenture.

#pragma once

#include "util/format/Printf.h"

namespace util
{
namespace format
{
enum class TokenType : uint8_t
{
    STRING,
    PARAM,
    END
};

/**
 * A class that scans a printf like format string. The string is split into tokens that can be
 * pulled one by one.
 *
 * There are two types of tokens: ordinary characters and conversions (starting with %) for
 * additional arguments. For each token both start and end pointer within the format string can
 * easily be retrieved. For conversions all information is written into a ParamInfo structure.
 *
 * The PrintfFormatScanner class will most probably be used to help processing an printf like output
 * operation. In this case there's the need to additionally retrieve the arguments which correspond
 * to the conversion. While most often only one argument is needed, there are two special types of
 * conversions ('*' for width or precision field) where there may be width or precision arguments
 * preceding the argument holding the value to convert. For these cases there are methods to simply
 * find out about the need for these special arguments and there are setters to store those values
 * to the ParamInfo structure which then can be passed by reference without the need of copying.
 */
class PrintfFormatScanner
{
public:
    /**
     * Default constructor.
     */
    PrintfFormatScanner();

    /**
     * Constructor to initialize the scanner with the format string.
     *
     * \param formatString C format string to scan
     */
    explicit PrintfFormatScanner(char const* formatString);

    /**
     * (Re)initialize the scanner with a format string.
     *
     * \param formatString C format string to scan
     */
    void init(char const* formatString);

    /**
     * Returns whether there are still tokens or not.
     *
     * \return
     * - true if there are one or more tokens (i.e. token type is not END)
     * - false otherwise
     */
    inline bool hasToken() const;
    /**
     * Scans the next token.
     */
    void nextToken();

    /**
     * Returns the type of the current token.
     *
     * \return
     * - STRING to indicate one or more ordinary characters within the format string that can
     *          simply be put out. The position of the first character can be retrieved with
     *          getTokenStart(), the position of the first character which is not part of the
     *          string with getTokenEnd().
     * - PARAM to indicate that there is a conversion (starting with %) on one or more
     *         arguments. All information about the conversion is translated into the fields
     *         of the ParamInfo structure that can be referenced with a call to
     *         getParamInfo(). The characters of the formatting string that represent this
     *         conversion can be retrieved in the same way as for ordinary character tokens.
     * - END to indicate there is no more token available i.e. the string has been scanned
     *          completely. Successive calls to nextToken() won't fail but will always return end
     *          tokens.
     */
    inline TokenType getTokenType() const;
    /**
     * Returns the position of the first character of the token.
     *
     * \return pointer to first character
     */
    inline char const* getTokenStart() const;
    /**
     * Returns the position of the first character which is not part of the token.
     *
     * \return pointer to first character behind the token. The length of the token can easily be
     *         calculated with getTokenEnd() - getTokenStart()
     */
    inline char const* getTokenEnd() const;

    /**
     * Returns all information about a single parameter conversion.
     * \note The content of this structure will not be touched unless there is a token of type
     * PARAM. Therefore reading out this structure before means accessing uninitialized
     * content!
     *
     * \return reference to the ParamInfo structure
     */
    inline ParamInfo const& getParamInfo() const;
    /**
     * Returns whether the current conversion needs the width field as an argument.
     *
     * \return
     * - true the width field is to be given by the first argument. The value can be set with
     * setWidth()
     * - false the width field is specified as a constant value within the format description
     */
    inline bool needsWidthParam() const;
    /**
     * Writes the width value into the fWidth field of the contained ParamInfo structure.
     *
     * \param width The desired value of the width field to set
     */
    inline void setWidth(int32_t width);
    /**
     * Returns whether the current conversion needs the precision field as an argument.
     *
     * \return
     * - true the precision field is to be given by the first argument (if needsWidthParam() returns
     * false) or the second argument (if needsWidthParam() returns true). The value can be set with
     * setPrecision()
     * - false the precision field is specified as a constant value within the format description
     */
    inline bool needsPrecisionParam() const;
    /**
     * Writes the precision value into the fPrecision field of the contained ParamInfo structure.
     *
     * \param precision The desired value of the precision field to set
     */
    inline void setPrecision(int32_t precision);

private:
    void scanString(uint32_t offset = 0U);
    void scanParam();
    int32_t scanWidthOrPrecision();

    void scanParamFlags();
    uint8_t scanParamLength();
    void scanParamFormatSpecifier(uint8_t intPower);

    inline void setParamType(ParamType type, ParamDatatype datatype, uint8_t base = 0U);
    inline void setIntParamType(uint8_t power, ParamDatatype byteDatatype, uint8_t base);

    static inline bool isDigit(char c);

    char const* _start;
    char const* _currentPosition;
    TokenType _tokenType;
    ParamInfo _paramInfo;
};

/* Implementation */
inline bool PrintfFormatScanner::hasToken() const { return _tokenType != TokenType::END; }

inline TokenType PrintfFormatScanner::getTokenType() const { return _tokenType; }

inline char const* PrintfFormatScanner::getTokenStart() const { return _start; }

inline char const* PrintfFormatScanner::getTokenEnd() const { return _currentPosition; }

inline ParamInfo const& PrintfFormatScanner::getParamInfo() const { return _paramInfo; }

inline bool PrintfFormatScanner::needsWidthParam() const
{
    return _paramInfo._width == ParamWidthOrPrecision::PARAM;
}

inline void PrintfFormatScanner::setWidth(int32_t const width) { _paramInfo._width = width; }

inline bool PrintfFormatScanner::needsPrecisionParam() const
{
    return _paramInfo._precision == ParamWidthOrPrecision::PARAM;
}

inline void PrintfFormatScanner::setPrecision(int32_t const precision)
{
    _paramInfo._precision = precision;
}

inline void PrintfFormatScanner::setParamType(
    ParamType const type, ParamDatatype const datatype, uint8_t const base)
{
    _paramInfo._type     = type;
    _paramInfo._base     = base;
    _paramInfo._datatype = datatype;
}

inline void PrintfFormatScanner::setIntParamType(
    uint8_t const power, ParamDatatype const byteDatatype, uint8_t const base)
{
    uint8_t const byteCast    = static_cast<uint8_t>(byteDatatype);
    uint8_t const next        = byteCast + power;
    ParamDatatype const param = static_cast<ParamDatatype>(next);
    setParamType(ParamType::INT, param, base);
}

inline bool PrintfFormatScanner::isDigit(char const c) { return (c >= '0') && (c <= '9'); }

} // namespace format
} // namespace util
