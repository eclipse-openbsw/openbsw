// Copyright 2024 Accenture.

#pragma once

#include "util/string/ConstString.h"

#include <cstdint>

namespace util
{
namespace format
{
/**
 * Enumeration defining main conversion types for printf.
 * \ingroup PrintfEnumerations
 */
enum class ParamType : uint8_t
{
    /// character conversion %c
    CHAR,
    /// string conversion: %s, %S
    STRING,
    /// pointer conversion: %p
    PTR,
    /// position pointer: %n
    POS,
    /// integer value conversions: %d, %i, %o, %x, %X
    INT
};

/**
 * Struct that holds flags to specify additional formatting hints.
 */
struct ParamFlags
{
    /// Indicates that values are aligned left within the size indicated by a width field
    static uint8_t const FLAG_LEFT    = 0x01U;
    /// Signed non-negative value are prefixed with '+'
    static uint8_t const FLAG_PLUS    = 0x02U;
    /// Signed non-negative values are prefixed with a space character.
    static uint8_t const FLAG_SPACE   = 0x04U;
    /// Chooses the 'alternate' output format for octal and hex values (prefix 0 resp. 0x/0X)
    static uint8_t const FLAG_ALT     = 0x08U;
    /// Fills numeric values with 0 up to the desired width
    static uint8_t const FLAG_ZEROPAD = 0x10U;
    /// Selects upper case characters for hex digits and prefixes
    static uint8_t const FLAG_UPPER   = 0x20U;
};

/**
 * Enumeration that represents all possible value types for printf arguments.
 * \ingroup PrintfEnumerations
 */
enum class ParamDatatype : uint8_t
{
    /// uint8_t
    UINT8,
    /// uint16_t
    UINT16,
    /// uint32_t
    UINT32,
    /// uint64_t
    UINT64,
    /// int8_t
    SINT8,
    /// int16_t
    SINT16,
    /// int32_t
    SINT32,
    /// int64_t
    SINT64,
    /// const void*
    VOIDPTR,
    /// const char*
    CHARPTR,
    /// const PlainSizedString*
    SIZEDCHARPTR,
    /// int32_t*
    SINT32PTR,

    COUNT
};

/**
 * Struct that holds special values for width or precision fields.
 */
struct ParamWidthOrPrecision
{
    /// Indicates that the default value (depending on the conversion type) for the field should be
    /// used
    static int32_t const DEFAULT = -1;
    /// Indicates that the width or precision will be specified by an additional argument value.
    static int32_t const PARAM   = -2;
};

/**
 * Simple structure that holds all printf information about a single conversion of a value argument.
 */
struct ParamInfo
{
    /// Contains the conversion type
    ParamType _type;
    /// A bit mask that contains additional formatting hints (see enum ParamFlags)
    uint8_t _flags;
    /// Holds the base (8, 10 or 16) for conversion of integer values
    uint8_t _base;
    /// The expected datatype for the argument values to convert
    ParamDatatype _datatype;
    /// Holds the desired width or one of the special values (see enum ParamWidthOrPrecision)
    int32_t _width;
    /// Holds the desired precision or one of the special values (see enum ParamWidthOrPrecision)
    int32_t _precision;
};

/**
 * Simple union that allows compact and typed storage of argument values needed for printf.
 * The enum ParamDatatype defines a value for each of the possible data fields
 */
union ParamVariant
{
    /// DATATYPE_UINT8
    uint8_t _uint8Value;
    /// DATATYPE_UINT16
    uint16_t _uint16Value;
    /// DATATYPE_UINT32
    uint32_t _uint32Value;
    /// DATATYPE_UINT64
    uint64_t _uint64Value;
    /// DATATYPE_SINT16
    int16_t _sint16Value;
    /// DATATYPE_SINT32
    int32_t _sint32Value;
    /// DATATYPE_SINT64
    int64_t _sint64Value;
    /// DATATYPE_VOIDPTR
    void const* _voidPtrValue;
    /// DATATYPE_CHARPTR
    char const* _charPtrValue;
    /// DATATYPE_SIZEDCHARPTR
    ::util::string::PlainSizedString const* _sizedCharPtrValue;
    /// DATATYPE_SINT32PTR
    int32_t* _sint32PtrValue;
};

} // namespace format
} // namespace util

