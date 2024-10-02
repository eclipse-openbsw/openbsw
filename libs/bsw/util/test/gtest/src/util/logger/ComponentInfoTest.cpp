// Copyright 2024 Accenture.

#include "util/logger/ComponentInfo.h"

#include <estd/assert.h>

#include <gtest/gtest.h>

using namespace ::util::logger;
using namespace ::util::format;

TEST(LoggerComponentInfoTest, testComponentInfo)
{
    {
        ComponentInfo cut;
        ASSERT_FALSE(cut.isValid());
    }
    ComponentInfo::PlainInfo plainInfo = {{"abc", {Color::YELLOW, BOLD, Color::BLACK}}};
    {
        ComponentInfo cut(12, &plainInfo);
        ASSERT_TRUE(cut.isValid());
        ASSERT_EQ(12, cut.getIndex());
        ASSERT_EQ("abc", std::string(cut.getName().getString()));
        ASSERT_EQ(
            StringAttributes(Color::YELLOW, BOLD, Color::BLACK), cut.getName().getAttributes());
    }
    {
        ComponentInfo src(12, &plainInfo);
        ComponentInfo cut = src;
        ASSERT_EQ(12, cut.getIndex());
        ASSERT_EQ("abc", std::string(cut.getName().getString()));
        ASSERT_EQ(
            StringAttributes(Color::YELLOW, BOLD, Color::BLACK), cut.getName().getAttributes());
    }
    {
        ComponentInfo src(12, &plainInfo);
        ComponentInfo cut;
        cut = src;
        ASSERT_EQ(12, cut.getIndex());
        ASSERT_EQ("abc", std::string(cut.getName().getString()));
        ASSERT_EQ(
            StringAttributes(Color::YELLOW, BOLD, Color::BLACK), cut.getName().getAttributes());
    }
    {
        ComponentInfo cut(12, &plainInfo);
        cut = *(&cut);
        ASSERT_EQ(12, cut.getIndex());
        ASSERT_EQ("abc", std::string(cut.getName().getString()));
        ASSERT_EQ(
            StringAttributes(Color::YELLOW, BOLD, Color::BLACK), cut.getName().getAttributes());
    }
    {
        ComponentInfo cut(12, &plainInfo);
        ASSERT_TRUE(cut.isValid());
        ASSERT_STREQ(plainInfo._nameInfo._string, cut.getPlainInfoString());
    }
}

TEST(LoggerComponentInfoTest, ComponentInfoInvalidPlainInfoCase)
{
    ComponentInfo cut(12);
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    {
        ASSERT_FALSE(cut.isValid());
    }
    {
        ASSERT_THROW(cut.getName(), ::estd::assert_exception);
    }
}

TEST(LoggerComponentInfoTest, ComponentInfoInvalidPlainInfoCaseForGetPlainInfoString)
{
    ComponentInfo cut(12);
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    {
        ASSERT_FALSE(cut.isValid());
    }
    {
        ASSERT_THROW(cut.getPlainInfoString(), ::estd::assert_exception);
    }
}
