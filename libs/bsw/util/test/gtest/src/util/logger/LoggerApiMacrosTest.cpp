// Copyright 2024 Accenture.

#include "util/logger/LoggerApiMacros.h"

#include <gtest/gtest.h>

namespace
{
TEST(LoggerApiMacrosTest, testAll)
{
    L_BOOL(true)
    L_I32(-1)
    L_U32(1U)
    L_H32(1U)
    L_I16(-1234)
    L_U16(1234U)
    L_H16(1234U)
    L_I32(-123423423)
    L_U32(12343433U)
    L_H32(12343433U)
    L_I64(-2134234823233)
    L_U64(2134234823233UL)
    L_H64(2134234823233UL)
    L_STR("abc")
    L_SIZED_STR(::util::string::ConstString("abc"))
    {
        bool const array[] = {true};
        L_ARRAY_BOOL(array)
    }
    {
        int8_t const array[] = {1, 2, 3};
        L_ARRAY_I8(array)
    }
    {
        uint8_t const array[] = {1U, 2U, 3U};
        L_ARRAY_U8(array)
        L_ARRAY_H8(array)
    }
    {
        int16_t const array[] = {1, 2, 3};
        L_ARRAY_I16(array)
    }
    {
        uint16_t const array[] = {1U, 2U, 3U};
        L_ARRAY_U16(array)
        L_ARRAY_H16(array)
    }
    {
        int32_t const array[] = {1, 2, 3};
        L_ARRAY_I32(array)
    }
    {
        uint32_t const array[] = {1U, 2U, 3U};
        L_ARRAY_U32(array)
        L_ARRAY_H32(array)
    }
    {
        int64_t const array[] = {1, 2, 3};
        L_ARRAY_I64(array)
    }
    {
        uint64_t const array[] = {1U, 2U, 3U};
        L_ARRAY_U64(array)
        L_ARRAY_H64(array)
    }

    L_NAME("abc", L_I64(1))
    L_UNIT("abc", L_I64(1))
    L_FMT("fmt", L_BOOL(true), L_BOOL(false));

    LOG_F(1U, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_F(1U, "msg", L_BOOL(true), L_BOOL(false));
    LOG_E(1U, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_E(1U, "msg", L_BOOL(true), L_BOOL(false));
    LOG_W(1U, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_W(1U, "msg", L_BOOL(true), L_BOOL(false));
    LOG_I(1U, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_I(1U, "msg", L_BOOL(true), L_BOOL(false));
    LOG_D(1U, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_D(1U, "msg", L_BOOL(true), L_BOOL(false));

    LOG_X(1U, LOG_LEVEL_OFF, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_X(1U, LOG_LEVEL_OFF, "msg", L_BOOL(true), L_BOOL(false));
    LOG_X(1U, LOG_LEVEL_FATAL, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_X(1U, LOG_LEVEL_FATAL, "msg", L_BOOL(true), L_BOOL(false));
    LOG_X(1U, LOG_LEVEL_ERROR, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_X(1U, LOG_LEVEL_ERROR, "msg", L_BOOL(true), L_BOOL(false));
    LOG_X(1U, LOG_LEVEL_WARN, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_X(1U, LOG_LEVEL_WARN, "msg", L_BOOL(true), L_BOOL(false));
    LOG_X(1U, LOG_LEVEL_INFO, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_X(1U, LOG_LEVEL_INFO, "msg", L_BOOL(true), L_BOOL(false));
    LOG_X(1U, LOG_LEVEL_DEBUG, "msg", L_BOOL(true), L_BOOL(false));
    LOG_FMT_X(1U, LOG_LEVEL_DEBUG, "msg", L_BOOL(true), L_BOOL(false));
}

} // anonymous namespace
