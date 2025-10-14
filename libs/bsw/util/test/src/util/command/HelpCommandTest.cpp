// Copyright 2024 Accenture.

#include "util/command/HelpCommand.h"

#include "util/command/ParentCommand.h"
#include "util/stream/SharedOutputStream.h"
#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::util::command;
using namespace ::util::stream;
using namespace ::util::string;

void dummyFunction(CommandContext& /* context */) {}

TEST(HelpCommandTest, testAll)
{
    SimpleCommand::ExecuteFunction dummyF
        = SimpleCommand::ExecuteFunction::create<&dummyFunction>();

    SimpleCommand leaf_1a_2a("leaf_1a_2a", "leaf_1a_2a desc.", dummyF);
    SimpleCommand leaf_1a_2b("leaf_1a_2b", "leaf_1a_2b desc.", dummyF);
    SimpleCommand leaf_1a_2c("leaf_1a_2c", nullptr, dummyF);

    ParentCommand level_1a("level_1a", "level_1a desc.");
    level_1a.addCommand(leaf_1a_2a);
    level_1a.addCommand(leaf_1a_2b);
    level_1a.addCommand(leaf_1a_2c);

    SimpleCommand leaf_1b_2b_3a("leaf_1b_2b_3a", "leaf_1b_2b_3a desc.", dummyF);
    ParentCommand level_1b_2b("level_1b_2b", "level_1b_2b desc.");
    level_1b_2b.addCommand(leaf_1b_2b_3a);

    SimpleCommand leaf_1b_2a("leaf_1b_2a", "leaf_1b_2a desc.", dummyF);
    SimpleCommand leaf_1b_2c("leaf_1b_2c", "leaf_1b_2c desc.", dummyF);

    ParentCommand level_1b("level_1b", "level_1b desc.");
    level_1b.addCommand(leaf_1b_2a);
    level_1b.addCommand(leaf_1b_2c);
    level_1b.addCommand(level_1b_2b);

    ParentCommand root("root", "root desc.");
    root.addCommand(level_1a);
    root.addCommand(level_1b);

    {
        HelpCommand cut(root);
        ASSERT_EQ(ConstString("help"), ConstString(cut.getId()));
    }
    {
        HelpCommand cut(root, 0U);
        ParentCommand parent("parent", "desc");
        parent.addCommand(cut);
        ASSERT_TRUE(!parent.getCommands().empty());
        ASSERT_EQ(ConstString("help"), ConstString(cut.getId()));
    }
    {
        HelpCommand cut(root, "id", "description");
        ASSERT_EQ(ConstString("id"), ConstString(cut.getId()));
        ASSERT_EQ(ConstString("description"), ConstString(cut.getDescription()));
    }
    {
        HelpCommand cut(root);
        ::util::stream::declare::StringBufferOutputStream<300> outputStream;
        SharedOutputStream sharedStream(outputStream);
        ASSERT_TRUE(cut.execute(ConstString(""), &sharedStream).isValid());
        ASSERT_EQ(
            ConstString("level_1a          - level_1a desc.\n"
                        "  leaf_1a_2a      - leaf_1a_2a desc.\n"
                        "  leaf_1a_2b      - leaf_1a_2b desc.\n"
                        "level_1b          - level_1b desc.\n"
                        "  leaf_1b_2a      - leaf_1b_2a desc.\n"
                        "  leaf_1b_2c      - leaf_1b_2c desc.\n"
                        "  level_1b_2b     - level_1b_2b desc.\n"
                        "    leaf_1b_2b_3a - leaf_1b_2b_3a desc.\n"),
            ConstString(outputStream.getString()));
    }
    {
        HelpCommand cut(root);
        ::util::stream::declare::StringBufferOutputStream<300> outputStream;
        SharedOutputStream sharedStream(outputStream);
        ASSERT_TRUE(cut.execute(ConstString("level_1b"), &sharedStream).isValid());
        ASSERT_EQ(
            ConstString("level_1b          - level_1b desc.\n"
                        "  leaf_1b_2a      - leaf_1b_2a desc.\n"
                        "  leaf_1b_2c      - leaf_1b_2c desc.\n"
                        "  level_1b_2b     - level_1b_2b desc.\n"
                        "    leaf_1b_2b_3a - leaf_1b_2b_3a desc.\n"),
            ConstString(outputStream.getString()));
    }
    {
        HelpCommand cut(root, 18U);
        ::util::stream::declare::StringBufferOutputStream<300> outputStream;
        SharedOutputStream sharedStream(outputStream);
        ASSERT_TRUE(cut.execute(ConstString("level_1b"), &sharedStream).isValid());
        ASSERT_EQ(
            ConstString("level_1b           - level_1b desc.\n"
                        "  leaf_1b_2a       - leaf_1b_2a desc.\n"
                        "  leaf_1b_2c       - leaf_1b_2c desc.\n"
                        "  level_1b_2b      - level_1b_2b desc.\n"
                        "    leaf_1b_2b_3a  - leaf_1b_2b_3a desc.\n"),
            ConstString(outputStream.getString()));
    }
    {
        SimpleCommand leaf1("leaf1", "leaf description\r \t also multiline", dummyF);
        ParentCommand parent("parent", "parent description\nmultiline\n");
        ParentCommand root("root", "root desc.");

        root.addCommand(parent);
        parent.addCommand(leaf1);

        HelpCommand cut(root);
        ::util::stream::declare::StringBufferOutputStream<300> outputStream;
        SharedOutputStream sharedStream(outputStream);
        ASSERT_TRUE(cut.execute(ConstString(""), &sharedStream).isValid());
        ASSERT_EQ(
            ConstString("parent  - parent description\n"
                        "          multiline\n"
                        "  leaf1 - leaf description\n"
                        "          also multiline\n"),
            ConstString(outputStream.getString()));
    }
    {
        HelpCommand cut(root);
        ::util::stream::declare::StringBufferOutputStream<300> stream;
        SharedOutputStream sharedStream(stream);
        ICommand::ExecuteResult result = cut.execute(ConstString("level_1a abc"), &sharedStream);
        ASSERT_EQ(ICommand::Result::BAD_VALUE, result.getResult());
        ASSERT_EQ(ConstString("abc"), result.getSuffix());
    }
}

} // anonymous namespace
