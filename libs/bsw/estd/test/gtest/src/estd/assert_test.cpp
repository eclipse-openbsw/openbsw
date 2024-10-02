// Copyright 2024 Accenture.

// Include class we are testing first to check for include dependencies
// #define ESTL_NO_ASSERT_FILE
#include "estd/assert.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

// crashes on Visual Studio
#ifndef _MSC_VER
TEST(AssertTest, TestDefaultHandler)
{
    ::estd::AssertHandlerScope scope(::estd::AssertDefaultHandler);
    //        ASSERT_DEATH_IF_SUPPORTED({estd_assert(0);}, "false");
}
#endif

TEST(AssertTest, TestException)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    try
    {
        estd_assert(0);
        ASSERT_TRUE(false); // should have thrown an exception
    }
    catch (::estd::assert_exception const& e)
    {
        ASSERT_STREQ("0", e.getTest());
        ASSERT_GE(__LINE__, e.getLine());
    }
}

TEST(AssertTest, TestExceptionFile)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    try
    {
        estd_assert(0);
        ASSERT_TRUE(false); // should have thrown an exception
    }
    catch (::estd::assert_exception const& e)
    {
        ASSERT_STREQ("0", e.getTest());

        // make sure that assert_test.cpp is part of the filename
        ASSERT_THAT(e.getFile(), HasSubstr(__FILE__));
#ifdef ESTD_HAS_EXCEPTIONS
        ASSERT_STREQ("0", e.what());
#endif
    }
}

TEST(AssertTest, NoAssertHandler)
{
    // turn off the assert handler
    ::estd::AssertHandlerScope scope(nullptr);

    // shouldn't crash!
    estd_assert(true);
    estd_assert(false);

    // test what happens when we don't have an assert handler
    ::estd::assert_func("file", 1, "test");

    {
        // now put an exception handler on the stack and make sure we throw an exception.
        ::estd::AssertHandlerScope except(::estd::AssertExceptionHandler);
        ASSERT_THROW(::estd::assert_func("file", 2, "test2"), ::estd::assert_exception);
    }
}

TEST(AssertTest, NoMacroTest)
{
    ::estd::AssertHandlerScope except(::estd::AssertExceptionHandler);

    // will throw an exception
    ASSERT_THROW(::estd::estd_assert_no_macro(false, "here", 1, "test"), ::estd::assert_exception);

    // should not throw since it isn't a failure
    ::estd::estd_assert_no_macro(true, "here", 1, "test");

    {
        ::estd::AssertHandlerScope scope(nullptr);
        // shouldn't do anything!
        ::estd::estd_assert_no_macro(false, "there", 2, "test1");
    }
}

TEST(AssertTest, EstdExcept)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);

    try
    {
        estd_expect(false);
        ASSERT_TRUE(false); // should have thrown an exception
    }
    catch (::estd::assert_exception const& e)
    {
        ASSERT_STREQ("false", e.getTest());
        ASSERT_GE(__LINE__, e.getLine());
    }
}

} // namespace
