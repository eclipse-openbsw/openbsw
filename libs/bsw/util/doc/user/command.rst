.. _util_command:

`util::command`
===============

Overview
--------

Typically, our embedded targets allow debug access through a simple `shell` over the console (serial
port). The `command shell` interface is part of the **util** module, allowing all **BSW**
modules to define commands for this `shell` without introducing additional dependencies.

The ``util::command::ICommand`` interface is designed for implementing custom
commands. However, it is rare for a custom command to implement this interface
directly. Typically, a custom command derives from or reuses one of the existing
implementations available within the command namespace.

SimpleCommand and ParentCommand
-------------------------------

A simple command can be easily created using the ``util::command::SimpleCommand`` class,
which can be constructed with all necessary parameters. No derivation is
required to implement a simple command. Its functionality can be encapsulated in a
(member) function and passed as a delegate via an ``etl::delegate`` object.

For groups or hierarchies of commands a ``util::command::ParentCommand`` object can be instantiated.
It is constructed with the necessary parameters (name and description) and allows child commands
to be added, which can be executed as subcommands. Arbitrary command hierarchies can be created by
nesting parent commands within each other.

Example
+++++++

The following code implements a simple command named "test" that includes two subcommands: "get"
and "put", which allow read and write access to a contained value.

The header file ``TestCommand.h`` could look like:

.. code-block:: cpp

    #include "util/command/ParentCommand.h"
    #include "util/command/SimpleCommand.h"
    #include "util/format/SharedStringWriter.h"

    namespace test
    {
    class TestCommand: public ::util::command::ParentCommand
    {
    public:
        TestCommand();

    private:
        void get(::util::command::CommandContext& context);
        void put(::util::command::CommandContext& context);

        ::util::command::SimpleCommand _get;
        ::util::command::SimpleCommand _put;

        uint32_t _value;
    };

    } // namespace test

and the corresponding source file:

.. code-block:: cpp

    #include "TestCommand.h"
    #include "util/format/SharedStringWriter.h"

    namespace test
    {

    using namespace ::util::command;
    using namespace ::util::format;
    using ExecuteFunction = ::etl::delegate<void(CommandContext& context)>;

    TestCommand::TestCommand()
    : ParentCommand("test", "Contains simple test commands.")
    , _get(
        "get",
        "Get the test value.",
        SimpleCommand::ExecuteFunction::create<TestCommand, &TestCommand::get>(*this),
        this
    )
    , _put("put",
        "Put the test value.",
        SimpleCommand::ExecuteFunction::create<TestCommand, &TestCommand::put>(*this),
        this
    )
    , _value(0)
    {
        addCommand(_get);
        addCommand(_put);
    }

    void
    TestCommand::get(CommandContext& context)
    {
        if (context.checkEol())
        {
            SharedStringWriter(context).printf("Value is %d", _value);
        }
    }

    void
    TestCommand::put(CommandContext& context)
    {
        uint32_t value = context.scanIntToken<uint32_t>();
        if (context.checkEol())
        {
            _value = value;
        }
    }

    } // namespace test

GroupCommand
------------

If minimizing RAM usage is a priority, you should use the ``GroupCommand`` class
as the base for your command functionality. This class replaces the ``bios::CommandInterpreter`` base class. The
``util::command::GroupCommand`` serves as an alternative to the
``util::command::ParentCommand`` class but supports only a two-level hierarchy (parent-child).

Example
+++++++

The file ``TestCommand.h`` counter part to the ``ParentCommand``/``SimpleCommand`` implementation of
TestCommand could then look like:

.. code-block:: cpp

    #include "util/command/GroupCommand.h"
    #include "util/format/SharedStringWriter.h"

    namespace test
    {
    class TestCommand : public GroupCommand
    {
    public:
        TestCommand() = default;

    protected:
        DECLARE_COMMAND_GROUP_GET_INFO
        void executeCommand(::util::command::CommandContext& context, uint8_t idx) override;

    private:
        enum Id
        {
            ID_GET,
            ID_PUT
        };

        uint32_t _value = 0;
    };

    } // namespace test

with the corresponding source file:

.. code-block:: cpp

    #include "TestCommand.h"
    #include "util/format/SharedStringWriter.h"

    namespace test
    {
    using namespace ::util::command;
    using namespace ::util::format;

    DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(TestCommand, "test", "Contains simple test commands.")
    COMMAND_GROUP_COMMAND(ID_GET, "get", "Get the test value.")
    COMMAND_GROUP_COMMAND(ID_PUT, "put", "Put the test value.")
    DEFINE_COMMAND_GROUP_GET_INFO_END

    // Implement the virtual function in the class:
    void TestCommand::executeCommand(CommandContext& context, uint8_t idx)
    {
        switch (idx)
        {
            case ID_GET:
                if (context.checkEol())
                {
                    SharedStringWriter(context).printf("Value is %d", _value);
                }
                break;
            case ID_PUT:
            {
                uint32_t value = context.scanIntToken<uint32_t>();
                if (context.checkEol())
                {
                    _value = value;
                }
            }
            break;
            default: break;
        }
    }

    } // namespace test


CommandContext
--------------

The default implementations heavily rely on the
``util::command::CommandContext`` class. Context objects of this type provide
access to command-line arguments and an output stream for presenting textual results.

Commands require a defined list of arguments. The ``util::command::CommandContext`` class offers helpful
methods for parsing arguments, such as identifiers, integer values, hex byte buffers, or arbitrary
whitespace-separated tokens. Additionally, it includes methods for checking conditions on the
arguments or determining if there are pending arguments. Refer to ``util::command::CommandContext``
for a more detailed description.
