// Copyright 2024 Accenture.

#include "util/estd/assert.h"

#include <platform/estdint.h>

#include <cstdlib>

#if defined(__linux__) || defined(__APPLE__)
#if __has_include("execinfo.h")
#include <execinfo.h>
#endif
#include <iostream>
#endif

namespace estd
{
static AssertHandler& global_assert_handler()
{
    static AssertHandler esr_global_assert_handler = &AssertDefaultHandler;
    return esr_global_assert_handler;
}

void estd_assert_no_macro(
    bool const condition, char const* const file, int const line, char const* const test)
{
    if (!condition)
    {
        if (global_assert_handler() != nullptr)
        {
            global_assert_handler()(file, line, test);
        }
    }
}

void assert_func(char const* const file, int const line, char const* const test)
{
    AssertHandler& handler = global_assert_handler();

    if (handler != nullptr)
    {
        handler(file, line, test);
    }
}

/*
 * Assert handlers
 */

void set_assert_handler(AssertHandler const handler) { global_assert_handler() = handler; }

AssertHandler get_assert_handler() { return global_assert_handler(); }

void AssertDefaultHandler(
#if defined(__linux__) || defined(__APPLE__)
    char const* const file, int const line, char const* const test)
#else
    char const* const /* file */, int const /* line */, char const* const /* test */)
#endif
{
#if defined(__linux__) || defined(__APPLE__)
#if __has_include("execinfo.h")
    void* callstack[128];
    backtrace_symbols_fd(callstack, backtrace(callstack, 128), 2 /* = stderr */);
#endif
    std::cout << "Assertion at " << file << ':' << line << " (" << test << ") failed" << std::endl;
#endif
    std::abort();
}

void AssertExceptionHandler(char const* const file, int const line, char const* const test)
{
#ifdef ESTD_HAS_EXCEPTIONS
    throw assert_exception(file, line, test);
#else
    (void)file;
    (void)line;
    (void)test;
#endif
}

/*
 * assert_exception
 */

#ifdef ESTD_HAS_EXCEPTIONS
assert_exception::assert_exception(
    char const* const file, int const line, char const* const test) throw()
: ::std::exception(), _file(file), _line(line), _test(test)
#else
assert_exception::assert_exception(char const* const file, int const line, char const* const test)
: _file(file), _line(line), _test(test)
#endif
{}

#ifdef ESTD_HAS_EXCEPTIONS
// virtual
char const* assert_exception::what() const throw() { return _test; }
#endif

char const* assert_exception::getFile() const { return _file; }

int assert_exception::getLine() const { return _line; }

char const* assert_exception::getTest() const { return _test; }

AssertHandlerScope::AssertHandlerScope(AssertHandler const next)
{
    _currentHandler = get_assert_handler();
    set_assert_handler(next);
}

AssertHandlerScope::~AssertHandlerScope() { set_assert_handler(_currentHandler); }
} // namespace estd
