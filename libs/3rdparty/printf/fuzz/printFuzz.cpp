// Copyright 2024 Accenture.

#include <fuzzing/fuzzing.h>

#include <estd/memory.h>

namespace test
{
// use functions in own test namespace to avoid stdio conflicts
#include "printf/printf.h"
} // namespace test

// dummy putchar
static char printf_buffer[1000];

void test::putchar_(char character) { printf_buffer[0] = character; }

void _out_fct(char character, void* arg)
{
    (void)arg;
    printf_buffer[0] = character;
}

namespace memory = ::estd::memory;

FUZZ(3rdpartyPrint, string_snprintf)
{
    char outputBuffer[1000] = {0};
    test::snprintf_(
        outputBuffer,
        sizeof(outputBuffer),
        fuzzing_input.reinterpret_as<char const>().data(),
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
