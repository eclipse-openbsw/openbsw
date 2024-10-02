// Copyright 2024 Accenture.

#include "util/command/ParentCommand.h"

#include "util/command/SimpleCommand.h"
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

struct ParentCommandTest
: public ::testing::Test
, public IParentCommand
, public ICommand::IHelpCallback
{
    ParentCommandTest() : _command(nullptr) {}

    void addCommand(ICommand& command) override { _command = &command; }

    void clearCommands() override { _command = nullptr; }

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

    static void staticFunc(CommandContext& context)
    {
        SharedStringWriter(context).printf("staticFunc: %d", context.scanIntToken<int32_t>());
    }

    void memberFunc(CommandContext& context)
    {
        SharedStringWriter(context).printf("memberFunc: %d", context.scanIntToken<int32_t>());
    }

    ICommand* _command;
    std::stringstream _stream;
};

TEST_F(ParentCommandTest, testInitialization)
{
    ParentCommand cut("id", "description");
    ASSERT_EQ(ConstString("id"), ConstString(cut.getId()));
    ASSERT_EQ(ConstString("description"), ConstString(cut.getDescription()));
    cut.getHelp(*this);
    ASSERT_EQ(
        std::string("start: id=id, desc=description, end=0\n"
                    "end\n"),
        getAndResetHelpString());
}

TEST_F(ParentCommandTest, testExecutionAndLookup)
{
    SimpleCommand staticCommand(
        "static",
        "Static Desc.",
        SimpleCommand::ExecuteFunction::create<&ParentCommandTest::staticFunc>());
    SimpleCommand memberCommand(
        "member",
        "Member Desc.",
        SimpleCommand::ExecuteFunction::create<ParentCommandTest, &ParentCommandTest::memberFunc>(
            *this));

    ParentCommand cut("id", "description");
    cut.addCommand(staticCommand);
    cut.addCommand(memberCommand);

    {
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString("Static 123"), &sharedStream);
        ASSERT_TRUE(result.isValid());
        ASSERT_EQ(ICommand::Result::OK, result.getResult());
        ASSERT_EQ(ConstString(""), result.getSuffix());
        ASSERT_EQ(&staticCommand, result.getCommand());
        ASSERT_EQ(ConstString("staticFunc: 123"), ConstString(stream.getString()));
    }
    {
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString(" memBer 123a"), &sharedStream);
        ASSERT_FALSE(result.isValid());
        ASSERT_EQ(ICommand::Result::BAD_TOKEN, result.getResult());
        ASSERT_EQ(ConstString("a"), result.getSuffix());
        ASSERT_EQ(&memberCommand, result.getCommand());
        ASSERT_EQ(ConstString("memberFunc: 0"), ConstString(stream.getString()));
    }
    {
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString(" other 123"), &sharedStream);
        ASSERT_FALSE(result.isValid());
        ASSERT_EQ(ICommand::Result::NOT_RESPONSIBLE, result.getResult());
        ASSERT_EQ(ConstString(" other 123"), result.getSuffix());
        ASSERT_EQ(nullptr, result.getCommand());
        ASSERT_EQ(ConstString(""), ConstString(stream.getString()));
    }
    {
        ICommand::ExecuteResult result = cut.execute(ConstString(" member 123"), nullptr);
        ASSERT_TRUE(result.isValid());
        ASSERT_EQ(ICommand::Result::OK, result.getResult());
        ASSERT_EQ(ConstString("123"), result.getSuffix());
        ASSERT_EQ(&memberCommand, result.getCommand());
    }
}

TEST_F(ParentCommandTest, testOrderedAccess)
{
    SimpleCommand memberCommand(
        "member",
        "Member Desc.",
        SimpleCommand::ExecuteFunction::create<ParentCommandTest, &ParentCommandTest::memberFunc>(
            *this));
    SimpleCommand staticCommand(
        "static",
        "Static Desc.",
        SimpleCommand::ExecuteFunction::create<&ParentCommandTest::staticFunc>());
    SimpleCommand otherCommand(
        "other",
        "Other Desc.",
        SimpleCommand::ExecuteFunction::create<&ParentCommandTest::staticFunc>());

    ParentCommand cut("id", "description");

    cut.addCommand(staticCommand);
    cut.addCommand(memberCommand);
    cut.addCommand(otherCommand);

    cut.getHelp(*this);

    ASSERT_EQ(
        std::string("start: id=id, desc=description, end=0\n"
                    "start: id=member, desc=Member Desc., end=1\n"
                    "start: id=other, desc=Other Desc., end=1\n"
                    "start: id=static, desc=Static Desc., end=1\n"
                    "end\n"),
        getAndResetHelpString());
}

TEST_F(ParentCommandTest, testClearCommand)
{
    SimpleCommand memberCommand(
        "member",
        "Member Desc.",
        SimpleCommand::ExecuteFunction::create<ParentCommandTest, &ParentCommandTest::memberFunc>(
            *this));
    SimpleCommand staticCommand(
        "static",
        "Static Desc.",
        SimpleCommand::ExecuteFunction::create<&ParentCommandTest::staticFunc>());
    SimpleCommand otherCommand(
        "other",
        "Other Desc.",
        SimpleCommand::ExecuteFunction::create<&ParentCommandTest::staticFunc>());

    ParentCommand cut("id", "description");

    cut.addCommand(staticCommand);
    cut.addCommand(memberCommand);
    cut.addCommand(otherCommand);

    cut.clearCommands();

    cut.getHelp(*this);

    ASSERT_EQ(
        std::string("start: id=id, desc=description, end=0\n"
                    "end\n"),
        getAndResetHelpString());
}

} // anonymous namespace
