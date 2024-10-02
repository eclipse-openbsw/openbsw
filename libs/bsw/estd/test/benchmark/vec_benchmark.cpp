// Copyright 2024 Accenture.

#define ESTL_HARD_ASSERT_FUNC ecu_assert
[[noreturn]] void ecu_assert();
#include "estd/vec.h"

#include <benchmark/benchmark.h>

/**
 * Helper class that increments a variable in its constructor and destructor. This class
 * is used to represent a complex data type in the benchmarks for vec.
 */
class TestCalls
{
public:
    TestCalls();

    ~TestCalls();

    TestCalls(TestCalls const&) = default;

    TestCalls& operator=(TestCalls const&) = default;

    TestCalls(TestCalls&&) = delete;

    TestCalls& operator=(TestCalls&&) = delete;
};

/**
 * \desc
 * Benchmark for creating an empty vec and calling assign to create max_size entries.
 */
template<class T>
void vec_assign(benchmark::State& state)
{
    double counter = 0;
    for (auto _ : state)
    {
        estd::vec<T, 1024> v;
        v.assign(v.max_size, T());
        ++counter;
    }
    state.counters["assign calls"] = ::benchmark::Counter(counter, benchmark::Counter::kIsRate);
}

BENCHMARK_TEMPLATE(vec_assign, TestCalls);
BENCHMARK_TEMPLATE(vec_assign, int);
BENCHMARK_TEMPLATE(vec_assign, uint8_t);

/**
 * \desc
 * Benchmark for creating an empty vec and calling assign to copy max_size elements from an input
 * range.
 */
template<class T>
void vec_assign_range(benchmark::State& state)
{
    double counter = 0;
    estd::vec<T, 1024> source(1024);
    for (auto _ : state)
    {
        estd::vec<T, 1024> v;
        v.assign(source.begin(), source.end());
        ++counter;
    }
    state.counters["assign calls"] = ::benchmark::Counter(counter, benchmark::Counter::kIsRate);
}

BENCHMARK_TEMPLATE(vec_assign_range, TestCalls);
BENCHMARK_TEMPLATE(vec_assign_range, int);
BENCHMARK_TEMPLATE(vec_assign_range, uint8_t);

/**
 * \desc
 * Benchmark for creating a full vec and clearing it afterwards.
 */
template<class T>
void vec_clear(benchmark::State& state)
{
    double counter = 0;
    for (auto _ : state)
    {
        estd::vec<T, 1024> v(1024);
        v.clear();
        ++counter;
    }
    state.counters["clear calls"] = ::benchmark::Counter(counter, benchmark::Counter::kIsRate);
}

BENCHMARK_TEMPLATE(vec_clear, TestCalls);
BENCHMARK_TEMPLATE(vec_clear, int);
BENCHMARK_TEMPLATE(vec_clear, uint8_t);

/**
 * \desc
 * Benchmark for creating an empty vec and calling resize to create max_size entries.
 */
template<class T>
void vec_resize(benchmark::State& state)
{
    double counter = 0;
    for (auto _ : state)
    {
        estd::vec<T, 1024> v;
        v.resize(v.max_size);
        ++counter;
    }
    state.counters["resize calls"] = ::benchmark::Counter(counter, benchmark::Counter::kIsRate);
}

BENCHMARK_TEMPLATE(vec_resize, TestCalls);
BENCHMARK_TEMPLATE(vec_resize, int);
BENCHMARK_TEMPLATE(vec_resize, uint8_t);

[[noreturn]] void ecu_assert()
{
    while (true) {}
}

TestCalls::TestCalls()
{
    static size_t volatile cnt = 0;
    ++cnt;
}

TestCalls::~TestCalls()
{
    static size_t volatile cnt = 0;
    ++cnt;
}
