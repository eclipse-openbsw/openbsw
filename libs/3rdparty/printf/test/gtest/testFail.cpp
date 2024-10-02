// Copyright 2024 Accenture.

#include <gtest/gtest.h>

#include <cstdlib>

namespace test
{
// use functions in own test namespace to avoid stdio conflicts
#include "printf/printf.h"
} // namespace test

using namespace ::testing;

void test::putchar_(char character) {}

TEST(ExtPrint, fail)
{
    char const fmtBuf[] = {'\x37', '\xB8', '\x25', '\x2E', '\x2A', '\x53', '\x57', '\x65', '\x2A',
                           '\xB8', '\x25', '\x2E', '\x2A', '\x65', '\x2A', '\x65', '\x2E', '\x2A',
                           '\x94', '\x25', '\xB8', '\x94', '\x94', '\x94', '\x94', '\x94', '\x94'};
    char out[1000];
    test::snprintf_(
        out,
        sizeof(out),
        fmtBuf,
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh");
}

TEST(ExtPrint, fail2)
{
    char const fmtBuf[] = "+&)&%.3333332lg%";
    char out[1000];
    test::snprintf_(
        out,
        sizeof(out),
        fmtBuf,
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh",
        "abcdefgh");
}
