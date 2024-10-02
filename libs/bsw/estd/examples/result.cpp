// Copyright 2024 Accenture.

#include "estd/result.h"

#include <platform/estdint.h>

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

// EXAMPLE_START return_from_function
namespace
{
struct Error
{
    uint32_t code;

    explicit Error(uint32_t code) : code(code) {}
};

struct Foo
{
    size_t bar;

    Foo(size_t bar = 42) : bar(bar) {}
};

struct Pod
{
    uint8_t t[10];
};
} // namespace

// EXAMPLE_END return_from_function

// EXAMPLE_START trait
enum class Problem
{
    NotEnoughChocolate,
    JustBadLuck_IGuess,
};

namespace estd
{
template<>
struct result_traits<Problem>
{
    static constexpr bool LIMITED = true;
};
} // namespace estd

// EXAMPLE_END trait
