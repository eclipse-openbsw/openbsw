// Copyright 2024 Accenture.

#include "util/command/SimpleCommand.h"

#include "util/command/IParentCommand.h"
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

struct SimpleCommandTest
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

    static void staticFunc(CommandContext& context)
    {
        SharedStringWriter(context).printf("staticFunc: %d", context.scanIntToken<int32_t>());
    }

    void memberFunc(CommandContext& context)
    {
        SharedStringWriter(context).printf("memberFunc: %d", context.scanIntToken<int32_t>());
    }

    std::stringstream _stream;
};

TEST_F(SimpleCommandTest, testInitializationAndExecution)
{
    {
        SimpleCommand cut(
            "id",
            "description",
            SimpleCommand::ExecuteFunction::create<&SimpleCommandTest::staticFunc>());
        ASSERT_EQ(ConstString("id"), ConstString(cut.getId()));
        ASSERT_EQ(ConstString("description"), ConstString(cut.getDescription()));
        cut.getHelp(*this);
        ASSERT_EQ(std::string("start: id=id, desc=description, end=1\n"), getAndResetHelpString());
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString("1235"), &sharedStream);
        ASSERT_TRUE(result.isValid());
        ASSERT_EQ(ICommand::Result::OK, result.getResult());
        ASSERT_EQ(ConstString(""), result.getSuffix());
        ASSERT_EQ(&cut, result.getCommand());
        ASSERT_EQ(ConstString("staticFunc: 1235"), ConstString(stream.getString()));
    }
    {
        SimpleCommand cut(
            "id",
            "description",
            SimpleCommand::ExecuteFunction::
                create<SimpleCommandTest, &SimpleCommandTest::memberFunc>(*this));
        ASSERT_EQ(ConstString("id"), ConstString(cut.getId()));
        ASSERT_EQ(ConstString("description"), ConstString(cut.getDescription()));
        cut.getHelp(*this);
        ASSERT_EQ(std::string("start: id=id, desc=description, end=1\n"), getAndResetHelpString());
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString("1235"), &sharedStream);
        ASSERT_TRUE(result.isValid());
        ASSERT_EQ(ICommand::Result::OK, result.getResult());
        ASSERT_EQ(ConstString(""), result.getSuffix());
        ASSERT_EQ(&cut, result.getCommand());
        ASSERT_EQ(ConstString("memberFunc: 1235"), ConstString(stream.getString()));
    }
}

TEST_F(SimpleCommandTest, testLookup)
{
    {
        SimpleCommand cut(
            "id",
            "description",
            SimpleCommand::ExecuteFunction::create<&SimpleCommandTest::staticFunc>());
        ICommand::ExecuteResult result = cut.execute(ConstString("1235"), nullptr);
        ASSERT_TRUE(result.isValid());
        ASSERT_EQ(ICommand::Result::OK, result.getResult());
        ASSERT_EQ(ConstString("1235"), result.getSuffix());
        ASSERT_EQ(&cut, result.getCommand());
    }
}

TEST_F(SimpleCommandTest, testErrorHandling)
{
    {
        SimpleCommand cut(
            "id",
            "description",
            SimpleCommand::ExecuteFunction::create<&SimpleCommandTest::staticFunc>());
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString("  123a"), &sharedStream);
        ASSERT_FALSE(result.isValid());
        ASSERT_EQ(ICommand::Result::BAD_TOKEN, result.getResult());
        ASSERT_EQ(ConstString("a"), result.getSuffix());
        ASSERT_EQ(&cut, result.getCommand());
        ASSERT_EQ(ConstString("staticFunc: 0"), ConstString(stream.getString()));
    }
}

} // anonymous namespace
