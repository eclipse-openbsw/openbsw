// Copyright 2024 Accenture.

#include "util/command/ICommand.h"

#include "util/command/ParentCommand.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::util::command;
using namespace ::util::string;

TEST(ICommandTest, testExecuteResult)
{
    {
        ICommand::ExecuteResult cut;
        ASSERT_TRUE(cut.isValid());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
        ASSERT_TRUE(cut.getSuffix().isEmpty());
        ASSERT_EQ(nullptr, cut.getCommand());
    }
    {
        ParentCommand parent("", "");
        ICommand::ExecuteResult cut(ICommand::Result::ERROR, ConstString("suffix"), &parent);
        ASSERT_FALSE(cut.isValid());
        ASSERT_EQ(ICommand::Result::ERROR, cut.getResult());
        ASSERT_EQ(ConstString("suffix"), cut.getSuffix());
        ASSERT_EQ(&parent, cut.getCommand());
        {
            ICommand::ExecuteResult cut2(cut);
            ASSERT_FALSE(cut2.isValid());
            ASSERT_EQ(ICommand::Result::ERROR, cut2.getResult());
            ASSERT_EQ(ConstString("suffix"), cut2.getSuffix());
            ASSERT_EQ(&parent, cut2.getCommand());
        }
        {
            ICommand::ExecuteResult cut2;
            cut2       = cut;
            auto& cut3 = cut2;
            cut2       = cut3;
            ASSERT_FALSE(cut2.isValid());
            ASSERT_EQ(ICommand::Result::ERROR, cut2.getResult());
            ASSERT_EQ(ConstString("suffix"), cut2.getSuffix());
            ASSERT_EQ(&parent, cut2.getCommand());
        }
    }
}

} // anonymous namespace
