// Copyright 2024 Accenture.

#include "estd/bitset.h"
#include "estd/math.h"
#include "estd/string.h"

#include <benchmark/benchmark.h>

struct MathBenchmark : public ::benchmark::Fixture
{};

// abs
void abs_int8_to_uint8(benchmark::State& state)
{
    int8_t const number = -1;
    uint64_t sum        = 0;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(sum += ::estd::abs<uint8_t>(number));
    }
}

BENCHMARK(abs_int8_to_uint8);

void abs_int16_to_uint16(benchmark::State& state)
{
    int16_t const number = -1;
    uint64_t sum         = 0;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(sum += ::estd::abs<uint16_t>(number));
    }
}

BENCHMARK(abs_int16_to_uint16);

void abs_int32_to_uint32(benchmark::State& state)
{
    int32_t const number = -1;
    uint64_t sum         = 0;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(sum += ::estd::abs<uint32_t>(number));
    }
}

BENCHMARK(abs_int32_to_uint32);

void abs_int64_to_uint64(benchmark::State& state)
{
    int64_t const number = -1;
    uint64_t sum         = 0;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(sum += ::estd::abs<uint64_t>(number));
    }
}

BENCHMARK(abs_int64_to_uint64);

void big_abs_int64_to_uint64(benchmark::State& state)
{
    int64_t const number = -9999999;
    uint64_t sum         = 0;
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(sum += ::estd::abs<uint64_t>(number));
    }
}

BENCHMARK(big_abs_int64_to_uint64);

// convert
void convert_1_uint8_t_base_10(benchmark::State& state)
{
    auto const numbers = ::estd::make_str("1");
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint8_t>(numbers, 10));
    }
}

BENCHMARK(convert_1_uint8_t_base_10);

void convert_2_uint8_t_base_10(benchmark::State& state)
{
    auto const numbers = ::estd::make_str("24");
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint8_t>(numbers, 10));
    }
}

BENCHMARK(convert_2_uint8_t_base_10);

void convert_1_uint8_t_base_16(benchmark::State& state)
{
    auto const numbers = ::estd::make_str("1");
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint8_t>(numbers, 16));
    }
}

BENCHMARK(convert_1_uint8_t_base_16);

void convert_2_uint8_t_base_16(benchmark::State& state)
{
    auto const numbers = ::estd::make_str("AB");
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint8_t>(numbers, 16));
    }
}

BENCHMARK(convert_2_uint8_t_base_16);

void convert_1_uint64_t_base_10(benchmark::State& state)
{
    uint8_t const bytes[] = {'1'};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint64_t>(bytes, 10));
    }
}

BENCHMARK(convert_1_uint64_t_base_10);

void convert_10_negative_int64_t_base_10(benchmark::State& state)
{
    uint8_t const bytes[] = {'-', '2', '3', '4', '5', '6', '7', '8', '9', '0'};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<int64_t>(bytes, 10));
    }
}

BENCHMARK(convert_10_negative_int64_t_base_10);

void convert_15_uint64_t_base_10(benchmark::State& state)
{
    uint8_t const bytes[]
        = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5'};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint64_t>(bytes, 10));
    }
}

BENCHMARK(convert_15_uint64_t_base_10);

void convert_1_uint64_t_base_16(benchmark::State& state)
{
    uint8_t const bytes[] = {'B'};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint64_t>(bytes, 16));
    }
}

BENCHMARK(convert_1_uint64_t_base_16);

void convert_10_uint64_t_base_16(benchmark::State& state)
{
    uint8_t const bytes[] = {'1', 'f', 'e', '4', 'F', 'E', 'd', 'C', 'B', 'A'};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint64_t>(bytes, 16));
    }
}

BENCHMARK(convert_10_uint64_t_base_16);

void convert_15_uint64_t_base_16(benchmark::State& state)
{
    uint8_t const bytes[]
        = {'1', 'd', 'e', 'f', 'F', 'E', 'D', 'C', 'B', 'A', 'a', 'b', '3', '4', '5'};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint64_t>(bytes, 16));
    }
}

BENCHMARK(convert_15_uint64_t_base_16);

void convert_15_uppercase_uint64_t_base_16(benchmark::State& state)
{
    uint8_t const bytes[]
        = {'A', 'D', 'E', 'F', 'F', 'E', 'D', 'C', 'B', 'A', 'A', 'B', 'F', 'D', 'C'};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint64_t>(bytes, 16));
    }
}

BENCHMARK(convert_15_uppercase_uint64_t_base_16);

void convert_15_lowercase_uint64_t_base_16(benchmark::State& state)
{
    uint8_t const bytes[]
        = {'a', 'd', 'e', 'f', 'f', 'e', 'd', 'c', 'b', 'a', 'a', 'b', 'f', 'd', 'c'};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint64_t>(bytes, 16));
    }
}

BENCHMARK(convert_15_lowercase_uint64_t_base_16);

void convert_60_uint64_t_base_2(benchmark::State& state)
{
    uint8_t const bytes[]
        = {'1', '0', '1', '0', '1', '1', '0', '1', '1', '1', '0', '0', '0', '1', '0',
           '1', '1', '1', '0', '1', '0', '0', '1', '1', '1', '0', '0', '0', '1', '0',
           '1', '0', '1', '0', '1', '1', '0', '0', '1', '1', '1', '0', '0', '1', '0',
           '1', '0', '1', '0', '1', '1', '0', '1', '1', '0', '0', '0', '0', '0', '0'};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(::estd::strtonum<uint64_t>(bytes, 2));
    }
}

BENCHMARK(convert_60_uint64_t_base_2);

// Benchmark Bitset assignment

void bitset_assign_value(benchmark::State& state)
{
    ::estd::array<uint64_t, 1> value;
    value[0] = 0xF0E1D2C3B4A5F6E7;
    ::estd::bitset<62> bitset;
    for (auto _ : state)
    {
        bitset.assign(value);
    }
}

BENCHMARK(bitset_assign_value);

void bitset_from_bytes(benchmark::State& state)
{
    ::estd::array<uint8_t, 8> value = {0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5, 0xF6, 0xE7};
    ::estd::bitset<62> bitset;
    for (auto _ : state)
    {
        bitset.from_bytes(value);
    }
}

BENCHMARK(bitset_from_bytes);

void bitset_from_smaller_bytes(benchmark::State& state)
{
    ::estd::array<uint8_t, 4> value;
    value.fill(0xFF);
    ::estd::bitset<62> bitset;
    for (auto _ : state)
    {
        bitset.reset();
        bitset.from_bytes(value);
    }
}

BENCHMARK(bitset_from_smaller_bytes);

void bitset_from_bigger_bytes(benchmark::State& state)
{
    ::estd::array<uint8_t, 16> value;
    value.fill(0xFF);
    ::estd::bitset<62> bitset;
    for (auto _ : state)
    {
        bitset.reset();
        bitset.from_bytes(value);
    }
}

BENCHMARK(bitset_from_bigger_bytes);

// Benchmark for SWAR bitcount algorithm
void bitset_count(benchmark::State& state)
{
    ::estd::bitset<128> b;

    b.set(1);
    b.set(2);
    b.set(4);
    uint32_t index = 1;
    for (auto _ : state)
    {
        b.set(index % 128);
        b.count();
        b.reset(index % 128);
        index += 7;
    }
}

BENCHMARK(bitset_count);
