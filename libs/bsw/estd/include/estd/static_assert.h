// Copyright 2024 Accenture.

/**
 * Contains STATIC_ASSERT_FAILURE macros
 * \file
 * \ingroup estl_utils
 */
#pragma once

#define ESTD_STATIC_CONCAT(X, Y)  X##Y
#define ESTD_STATIC_CONCAT2(X, Y) ESTD_STATIC_CONCAT(X, Y)

#if (__cplusplus > 199711L) // static_assert supported since c++11

#define ESTD_STATIC_ASSERT(CONDITION)                  static_assert(CONDITION, "ESTD_STATIC_ASSERT")
#define ESTD_STATIC_ASSERT_MESSAGE(CONDITION, MESSAGE) static_assert(CONDITION, MESSAGE)

#else

namespace estd
{
namespace internal
{
template<bool B>
struct make_bool
{
    static bool const value = B;
};
} // namespace internal

template<bool>
struct STATIC_ASSERT_FAILURE;

template<>
struct STATIC_ASSERT_FAILURE<true>
{};

} // namespace estd

#define ESTD_STATIC_ASSERT_(CONDITION)                                                           \
    enum                                                                                         \
    {                                                                                            \
        ESTD_STATIC_CONCAT2(static_assert_failure_, __LINE__)                                    \
        = sizeof(::estd::STATIC_ASSERT_FAILURE<::estd::internal::make_bool<(CONDITION)>::value>) \
    }

#define ESTD_STATIC_ASSERT(CONDITION)                  ESTD_STATIC_ASSERT_(CONDITION)
#define ESTD_STATIC_ASSERT_MESSAGE(CONDITION, MESSAGE) ESTD_STATIC_ASSERT_(CONDITION)

#endif

/**
 * \deprecated  Use ESTD_STATIC_ASSERT instead.
 */
#define ESTD_STATIC_ASSERT_IN_FUNCTION(CONDITION) ESTD_STATIC_ASSERT(CONDITION)
