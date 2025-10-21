// Copyright 2024 Accenture.

#include "app/app.h"

#include <etl/error_handler.h>

#if defined(__linux__) || defined(__APPLE__)
#include <cstdlib>
#include <iostream>
#if __has_include("execinfo.h")
#include <execinfo.h>
#endif

void etl_assert_function(etl::exception const& exception)
{
#if __has_include("execinfo.h")
    void* callstack[128];
    backtrace_symbols_fd(callstack, backtrace(callstack, 128), 2 /* = stderr */);
#endif
    std::cout << "Assertion at " << exception.file_name() << ':' << exception.line_number() << " ("
              << exception.what() << ") failed" << std::endl;
    std::abort();
}
#else // defined(__linux__) || defined(__APPLE__)
#include <cstdlib>

void etl_assert_function(etl::exception const&) { std::abort(); }
#endif

void app_main()
{
    etl::set_assert_function(etl_assert_function);
    ::app::run();
}
