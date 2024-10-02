// Copyright 2024 Accenture.

#include "util/format/PrintfArgumentReader.h"

#include <gtest/gtest.h>

using namespace ::util::format;

namespace
{
template<typename PointerType>
static PointerType toPointer(uintptr_t const address)
{
    return reinterpret_cast<PointerType>(address);
}

ParamVariant const& safeReadArgument(PrintfArgumentReader& reader, ParamDatatype datatype)
{
    ParamVariant const* pVariant = reader.readArgument(datatype);
    return *pVariant;
}

void readArguments(char const* pDummy, ...)
{
    va_list ap;
    va_start(ap, pDummy);
    PrintfArgumentReader cut(ap);
    va_end(ap);
    ASSERT_EQ(1U, safeReadArgument(cut, ParamDatatype::SINT8)._uint8Value);
    ASSERT_EQ(2U, safeReadArgument(cut, ParamDatatype::UINT8)._uint8Value);
    ASSERT_EQ(3, safeReadArgument(cut, ParamDatatype::SINT16)._sint16Value);
    ASSERT_EQ(4U, safeReadArgument(cut, ParamDatatype::UINT16)._uint16Value);
    ASSERT_EQ(5, safeReadArgument(cut, ParamDatatype::SINT32)._sint32Value);
    ASSERT_EQ(6U, safeReadArgument(cut, ParamDatatype::UINT32)._uint32Value);
    ASSERT_EQ(7, safeReadArgument(cut, ParamDatatype::SINT64)._sint64Value);
    ASSERT_EQ(8U, safeReadArgument(cut, ParamDatatype::UINT64)._uint64Value);
    ASSERT_EQ(
        toPointer<void const*>(9), safeReadArgument(cut, ParamDatatype::VOIDPTR)._voidPtrValue);
    ASSERT_EQ(
        toPointer<char const*>(10), safeReadArgument(cut, ParamDatatype::CHARPTR)._charPtrValue);
    ASSERT_EQ(
        toPointer<int32_t*>(11), safeReadArgument(cut, ParamDatatype::SINT32PTR)._sint32PtrValue);
    ASSERT_EQ(0U, safeReadArgument(cut, ParamDatatype::COUNT)._uint64Value);
}

} // anonymous namespace

TEST(PrintfArgumentReaderTest, testRead)
{
    readArguments(
        "",
        (int8_t)1,
        (uint8_t)2,
        (int16_t)3,
        (uint16_t)4,
        (int32_t)5,
        (uint32_t)6,
        (int64_t)7,
        (uint64_t)8,
        toPointer<void const*>(9),
        toPointer<char const*>(10),
        toPointer<int32_t*>(11));
}
