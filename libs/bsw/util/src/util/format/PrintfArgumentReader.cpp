// Copyright 2024 Accenture.

#include "util/format/PrintfArgumentReader.h"

namespace util
{
namespace format
{
PrintfArgumentReader::PrintfArgumentReader(::estd::va_list_ref const ap) : _ap(ap), _variant() {}

ParamVariant const* PrintfArgumentReader::readArgument(ParamDatatype const datatype)
{
    switch (datatype)
    {
        case ParamDatatype::UINT8:
        case ParamDatatype::SINT8:
        {
            va_list& vaList = _ap.get();

            _variant._uint8Value = static_cast<uint8_t>(va_arg(vaList, uint32_t));
            break;
        }
        case ParamDatatype::UINT16:
        case ParamDatatype::SINT16:
        {
            va_list& vaList = _ap.get();

            _variant._uint16Value = static_cast<uint16_t>(va_arg(vaList, uint32_t));
            break;
        }
        case ParamDatatype::UINT32:
        case ParamDatatype::SINT32:
        {
            va_list& vaList = _ap.get();

            _variant._uint32Value = va_arg(vaList, uint32_t);
            break;
        }
        case ParamDatatype::UINT64:
        case ParamDatatype::SINT64:
        {
            va_list& vaList = _ap.get();

            _variant._uint64Value = va_arg(vaList, uint64_t);
            break;
        }
        case ParamDatatype::VOIDPTR:
        case ParamDatatype::CHARPTR:
        case ParamDatatype::SIZEDCHARPTR:
        case ParamDatatype::SINT32PTR:
        {
            va_list& vaList = _ap.get();

            _variant._voidPtrValue = va_arg(vaList, void*);
            break;
        }
        default:
        {
            _variant._uint64Value = 0U;
            break;
        }
    }
    return &_variant;
}

} // namespace format
} // namespace util
