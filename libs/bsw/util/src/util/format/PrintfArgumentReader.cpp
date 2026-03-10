// Copyright 2024 Accenture.

#include "util/format/PrintfArgumentReader.h"

namespace util
{
namespace format
{
PrintfArgumentReader::PrintfArgumentReader(::estd::va_list_ref const ap) : _ap(ap), _variant() {}

ParamVariant const* PrintfArgumentReader::readArgument(ParamDatatype const datatype)
{
    // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access): intentional tagged-by-ParamDatatype
    // union access

    switch (datatype)
    {
        case ParamDatatype::UINT8:
        case ParamDatatype::SINT8:
        {
            va_list& vaList      = _ap.get();
            _variant._uint8Value = static_cast<uint8_t>(
                va_arg(vaList, uint32_t)); // NOLINT(cppcoreguidelines-pro-type-vararg): required
                                           // interface for printf-style args
            break;
        }
        case ParamDatatype::UINT16:
        case ParamDatatype::SINT16:
        {
            va_list& vaList       = _ap.get();
            _variant._uint16Value = static_cast<uint16_t>(
                va_arg(vaList, uint32_t)); // NOLINT(cppcoreguidelines-pro-type-vararg): required
                                           // interface for printf-style args
            break;
        }
        case ParamDatatype::UINT32:
        case ParamDatatype::SINT32:
        {
            va_list& vaList = _ap.get();
            _variant._uint32Value
                = va_arg(vaList, uint32_t); // NOLINT(cppcoreguidelines-pro-type-vararg): required
                                            // interface for printf-style args
            break;
        }
        case ParamDatatype::UINT64:
        case ParamDatatype::SINT64:
        {
            va_list& vaList = _ap.get();
            _variant._uint64Value
                = va_arg(vaList, uint64_t); // NOLINT(cppcoreguidelines-pro-type-vararg): required
                                            // interface for printf-style args
            break;
        }
        case ParamDatatype::VOIDPTR:
        case ParamDatatype::CHARPTR:
        case ParamDatatype::SIZEDCHARPTR:
        case ParamDatatype::SINT32PTR:
        {
            va_list& vaList = _ap.get();
            _variant._voidPtrValue
                = va_arg(vaList, void*); // NOLINT(cppcoreguidelines-pro-type-vararg): required
                                         // interface for printf-style args
            break;
        }
        default:
        {
            _variant._uint64Value = 0U;
            break;
        }
    }
    // NOLINTEND(cppcoreguidelines-pro-type-union-access)
    return &_variant;
}

} // namespace format
} // namespace util
