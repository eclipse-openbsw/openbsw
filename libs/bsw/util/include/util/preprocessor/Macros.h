// Copyright 2024 Accenture.

#pragma once

// clang-format off
#define ESR_1(WHAT, X)      WHAT(X)
#define ESR_2(WHAT, X, ...) WHAT(X); ESR_1(WHAT, __VA_ARGS__);
#define ESR_3(WHAT, X, ...) WHAT(X); ESR_2(WHAT, __VA_ARGS__);
#define ESR_4(WHAT, X, ...) WHAT(X); ESR_3(WHAT, __VA_ARGS__);
#define ESR_5(WHAT, X, ...) WHAT(X); ESR_4(WHAT, __VA_ARGS__);
#define ESR_6(WHAT, X, ...) WHAT(X); ESR_5(WHAT, __VA_ARGS__);
#define ESR_7(WHAT, X, ...) WHAT(X); ESR_6(WHAT, __VA_ARGS__);
#define ESR_8(WHAT, X, ...) WHAT(X); ESR_7(WHAT, __VA_ARGS__);
#define ESR_9(WHAT, X, ...) WHAT(X); ESR_8(WHAT, __VA_ARGS__);
#define ESR_10(WHAT, X, ...) WHAT(X); ESR_9(WHAT, __VA_ARGS__);

#define ESR_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME

#define ESR_FOR_EACH(WHAT, ...) \
  ESR_GET_MACRO(__VA_ARGS__, ESR_10, ESR_9, ESR_8, ESR_7, ESR_6, ESR_5, ESR_4, ESR_3, ESR_2, ESR_1, ~)(WHAT, __VA_ARGS__)
// clang-format on

#define ESR_EVAL(WHAT) WHAT

#define ESR_SECOND(_1, _2, ...) _2

#define ESR_IS_PROBE(...) ESR_SECOND(__VA_ARGS__, 0)
#define ESR_PROBE()       ~, 1

#define ESR_CAT(_1, _2) _1##_2

#define ESR_NOT(X) ESR_IS_PROBE(ESR_CAT(ESR_NOT_, X))
#define ESR_NOT_0  ESR_PROBE()

#define ESR_BOOL(X) ESR_NOT(ESR_NOT(X))

#define ESR_IF_ELSE(CONDITION)   ESR_IF_ELSE_P(ESR_BOOL(CONDITION))
#define ESR_IF_ELSE_P(CONDITION) ESR_CAT(ESR_IF_, CONDITION)

#define ESR_IF_1(...) __VA_ARGS__ ESR_IF_1_ELSE
#define ESR_IF_0(...) ESR_IF_0_ELSE

#define ESR_IF_1_ELSE(...)
#define ESR_IF_0_ELSE(...) __VA_ARGS__
