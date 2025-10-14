// Copyright 2024 Accenture.

#include "util/command/GroupCommand.h"

#include "util/format/SharedStringWriter.h"
#include "util/stream/SharedOutputStream.h"
#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

#include <sstream>

namespace
{
using namespace ::util::command;
using namespace ::util::format;
using namespace ::util::stream;
using namespace ::util::string;

struct GroupCommandTest
: public ::testing::Test
, public ICommand::IHelpCallback
{
    void startCommand(char const* id, char const* description, bool end) override
    {
        _stream << "start: id=" << id << ", desc=" << description << ", end=" << end << "\n";
    }

    void endCommand() override { _stream << "end\n"; }

    std::string getAndResetHelpString()
    {
        std::string str = _stream.str();
        _stream.str("");
        return str;
    }

    std::stringstream _stream;
};

struct TestGroupCommand : GroupCommand
{
    DECLARE_COMMAND_GROUP_GET_INFO

    void executeCommand(CommandContext& context, uint8_t idx) override
    {
        SharedStringWriter(context).printf("cmd = %d\n", idx);
        while (context.hasToken())
        {
            SharedStringWriter(context).printf("arg = %d\n", context.scanIntToken<uint32_t>());
        }
    }
};

DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(TestGroupCommand, "group", "group desc.")
COMMAND_GROUP_COMMAND(2, "cmd2", "desc 2")
COMMAND_GROUP_COMMAND(5, "cmd5", "desc 5")
COMMAND_GROUP_COMMAND(7, "675", "desc 7")
DEFINE_COMMAND_GROUP_GET_INFO_END

TEST_F(GroupCommandTest, testInitialization)
{
    TestGroupCommand cut;
    ASSERT_EQ(ConstString("group"), ConstString(cut.getId()));
    ASSERT_EQ(ConstString("group desc."), ConstString(cut.getDescription()));
    cut.getHelp(*this);
    ASSERT_EQ(
        std::string("start: id=group, desc=group desc., end=0\n"
                    "start: id=cmd2, desc=desc 2, end=1\n"
                    "start: id=cmd5, desc=desc 5, end=1\n"
                    "start: id=675, desc=desc 7, end=1\n"
                    "end\n"),
        getAndResetHelpString());
}

TEST_F(GroupCommandTest, testExecutionAndLookup)
{
    TestGroupCommand cut;
    {
        ::util::stream::declare::StringBufferOutputStream<100> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString(" Cmd2 1 2"), &sharedStream);
        ASSERT_TRUE(result.isValid());
        ASSERT_EQ(ICommand::Result::OK, result.getResult());
        ASSERT_EQ(ConstString(""), result.getSuffix());
        ASSERT_EQ(&cut, result.getCommand());
        ASSERT_EQ(ConstString("cmd = 2\narg = 1\narg = 2\n"), ConstString(stream.getString()));
    }
    {
        ::util::stream::declare::StringBufferOutputStream<100> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString(" 675 1 2"), &sharedStream);
        ASSERT_TRUE(result.isValid());
        ASSERT_EQ(ICommand::Result::OK, result.getResult());
        ASSERT_EQ(ConstString(""), result.getSuffix());
        ASSERT_EQ(ConstString("cmd = 7\narg = 1\narg = 2\n"), ConstString(stream.getString()));
    }
    {
        ICommand::ExecuteResult result = cut.execute(ConstString(" Cmd2 1 2"), nullptr);
        ASSERT_TRUE(result.isValid());
        ASSERT_EQ(ICommand::Result::OK, result.getResult());
        ASSERT_EQ(ConstString(" Cmd2 1 2"), result.getSuffix());
    }
    {
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString(" other 123"), &sharedStream);
        ASSERT_FALSE(result.isValid());
        ASSERT_EQ(ICommand::Result::NOT_RESPONSIBLE, result.getResult());
        ASSERT_EQ(ConstString(" other 123"), result.getSuffix());
        ASSERT_EQ(ConstString(""), ConstString(stream.getString()));
    }
}

} // anonymous namespace
