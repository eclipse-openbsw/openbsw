// Copyright 2024 Accenture.

/**
 * Contains assert macros and classes.
 * \file
 * \ingroup estl_utils
 */
#pragma once

#ifdef ESTD_HAS_EXCEPTIONS
#include <exception> // IWYU pragma: export
#endif

#ifndef ESTD_UNREACHABLE
#if defined(__GNUC__)
#define ESTD_UNREACHABLE (__builtin_unreachable())
#else
#ifdef __cplusplus
#define ESTD_UNREACHABLE (static_cast<void>(0))
#else
#define ESTD_UNREACHABLE ((void)0)
#endif // __cplusplus
#endif
#endif // ESTD_UNREACHABLE

#if defined(ESTL_HARD_ASSERT_FUNC)
#if defined(ESTL_HARD_ASSERT_HEADER)
#include ESTL_HARD_ASSERT_HEADER
#endif
#ifdef __cplusplus
#define estd_assert(E__) ((E__) ? static_cast<void>(0) : ESTL_HARD_ASSERT_FUNC())
#else
#define estd_assert(E__) ((E__) ? (void)0 : ESTL_HARD_ASSERT_FUNC())
#endif

#elif defined(ESTL_ASSERT_MESSAGE_ALL)
#define estd_assert(E__)          \
    ((E__) ? static_cast<void>(0) \
           : (::estd::assert_func(__FILE__, __LINE__, #E__), ESTD_UNREACHABLE))

#elif defined(ESTL_ASSERT_FUNCTION)
#define estd_assert(E__) ::estd::estd_assert_no_macro((E__), __FILE__, __LINE__, #E__)

#elif defined(ESTL_NO_ASSERT)
#ifdef __cplusplus
#define estd_assert(E__) (static_cast<void>(0))
#else
#define estd_assert(E__) ((void)0)
#endif
#endif

// if no assert style is picked then we will use the "default" one.
#ifndef estd_assert
#define estd_assert(E__)        \
    ((E__)                      \
         ? static_cast<void>(0) \
         : (::estd::assert_func(static_cast<char const*>(0L), __LINE__, #E__), ESTD_UNREACHABLE))
#endif

// estd_expect behaves the same as estd_assert, but it shall not be used in
// production code, only for temporary asserts during development.
// The usage of estd_expect can be detected by static code analyzers.
#if defined(ESTD_EXPECT_ERROR)
#define estd_expect(E__) static_assert(false, "Replace estd_expect with proper error handling")
#else
#define estd_expect(E__) estd_assert(E__)
#endif

#ifdef __cplusplus

namespace estd
{
extern void estd_assert_no_macro(
    bool /* condition */, char const* /* file */, int /* line */, char const* /* test */);

extern void assert_func(char const* /* file */, int /* line */, char const* /* test */);

/*
 * Assert Handler function
 */
using AssertHandler = void (*)(char const*, int, char const*);

extern void set_assert_handler(AssertHandler);

extern AssertHandler get_assert_handler();

/*
 * Two pre-defined assert handlers.
 */

/**
 * Default one. Just calls assert(false)
 */
extern void AssertDefaultHandler(char const* /* file */, int /* line */, char const* /* test */);

/**
 * This one throws the ::estd::assert_exception if
 * exceptions are enabled. Otherwise it calls assert(false).
 */
extern void AssertExceptionHandler(char const* /* file */, int /* line */, char const* /* test */);

/**
 * An exception that is thrown with the assert.
 */
class assert_exception
#ifdef ESTD_HAS_EXCEPTIONS
: std::exception
#endif
{
public:
#ifdef ESTD_HAS_EXCEPTIONS
    assert_exception(char const* file, int line, char const* test) throw();
#else
    assert_exception(char const* file, int line, char const* test);
#endif

#ifdef ESTD_HAS_EXCEPTIONS
    char const* what() const throw() override;
#endif
    char const* getFile() const;
    int getLine() const;
    char const* getTest() const;

private:
    char const* _file;
    int _line;
    char const* _test;
};

class AssertHandlerScope
{
public:
    explicit AssertHandlerScope(AssertHandler next);
    ~AssertHandlerScope();

private:
    AssertHandler _currentHandler;
};

} // namespace estd

#endif
