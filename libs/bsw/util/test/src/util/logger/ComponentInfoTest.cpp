/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/logger/ComponentInfo.h"

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
    {
        ASSERT_FALSE(cut.isValid());
    }
    {
        ASSERT_THROW(cut.getName(), ::etl::exception);
    }
}

TEST(LoggerComponentInfoTest, ComponentInfoInvalidPlainInfoCaseForGetPlainInfoString)
{
    ComponentInfo cut(12);
    {
        ASSERT_FALSE(cut.isValid());
    }
    {
        ASSERT_THROW(cut.getPlainInfoString(), ::etl::exception);
    }
}
