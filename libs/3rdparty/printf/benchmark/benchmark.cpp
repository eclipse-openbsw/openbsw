// Copyright 2024 Accenture.

#include <benchmark/benchmark.h>

namespace test
{
// use functions in own test namespace to avoid stdio conflicts
#include "printf/printf.h"
} // namespace test

// dummy putchar
static char printf_buffer[100];

void test::putchar_(char character) { printf_buffer[0] = character; }

void _out_fct(char character, void* arg)
{
    (void)arg;
    printf_buffer[0] = character;
}

struct PrintfBenchmark : public ::benchmark::Fixture
{};

static void BM_String(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%s";
        test::sprintf_(out, strFmt, "yolo");
    }
}

BENCHMARK(BM_String);

static void BM_Float(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%g";
        test::sprintf_(out, strFmt, 123.456);
    }
}

BENCHMARK(BM_Float);

static void BM_Char(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%c";
        test::sprintf_(out, strFmt, 'a');
    }
}

BENCHMARK(BM_Char);

static void BM_Uint8(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%hhu";
        test::sprintf_(out, strFmt, 10U);
    }
}

BENCHMARK(BM_Uint8);

static void BM_Uint16(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%hu";
        test::sprintf_(out, strFmt, 10U);
    }
}

BENCHMARK(BM_Uint16);

static void BM_Uint32(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%u";
        test::sprintf_(out, strFmt, 10U);
    }
}

BENCHMARK(BM_Uint32);

static void BM_Uint64(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%llu";
        test::sprintf_(out, strFmt, 10U);
    }
}

BENCHMARK(BM_Uint64);

static void BM_Int8(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%hhi";
        test::sprintf_(out, strFmt, 10);
    }
}

BENCHMARK(BM_Int8);

static void BM_Int16(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%hi";
        test::sprintf_(out, strFmt, 10);
    }
}

BENCHMARK(BM_Int16);

static void BM_Int32(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%i";
        test::sprintf_(out, strFmt, 10);
    }
}

BENCHMARK(BM_Int32);

static void BM_Int64(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const strFmt[] = "%lli";
        test::sprintf_(out, strFmt, 10);
    }
}

BENCHMARK(BM_Int64);

static void BM_Int_Precision_Zero_Pad(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const precisionFmt[] = "%.10d";
        test::sprintf_(out, precisionFmt, int(1));
    }
}

BENCHMARK(BM_Int_Precision_Zero_Pad);

static void BM_Int_Precision_Zero_Pad_Explicit(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const precisionZeroPadFmt[] = "%0.10d";
        test::sprintf_(out, precisionZeroPadFmt, int(1));
    }
}

BENCHMARK(BM_Int_Precision_Zero_Pad_Explicit);

static void BM_Int_Positive_Sign(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const plusFmt[] = "%+d";
        test::sprintf_(out, plusFmt, int(1));
    }
}

BENCHMARK(BM_Int_Positive_Sign);

static void BM_Int_Precision_Space_Pad(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const leftFmt[] = "%10d";
        test::sprintf_(out, leftFmt, int(1));
    }
}

BENCHMARK(BM_Int_Precision_Space_Pad);

static void BM_Int_Left_Zero_Pad_Explicit(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const leftZeroFmt[] = "%010d";
        test::sprintf_(out, leftZeroFmt, int(1));
    }
}

BENCHMARK(BM_Int_Left_Zero_Pad_Explicit);

static void BM_Int_Right_Zero_Pad_Explicit(benchmark::State& state)
{
    char out[128];
    for (auto _ : state)
    {
        char const rightFmt[] = "%-10d";
        test::sprintf_(out, rightFmt, int(1));
    }
}

BENCHMARK(BM_Int_Right_Zero_Pad_Explicit);
