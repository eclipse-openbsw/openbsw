.. _util_command:

command - Command Framework
===========================

Overview command Module
-----------------------

Typically our embedded targets are allowing debug access via a simple shell over the console (serial
port). The new command shell interface is located in the util project and thus allows all bsw
modules to simply define commands for this shell without adding further dependencies.

The interface ``::util::command::ICommand`` is dedicated for implementing such custom
commands. But there are only rare cases when a custom command needs to implement this interface
directly. Typically a custom command will derive or reuse one of the existing implementations that
can be found within the command namespace.

SimpleCommand and ParentCommand
-------------------------------

A simple command can be easily built by using the class ``::util::command::SimpleCommand``
which can be constructed with all necessary parameters. No derivation is needed to implement a
simple command, the functionality of this command can easily be packed into a (member) method and
given as a delegate via a estd function object.

For groups or hierarchies of commands a ``ParentCommand`` object can be instantiated. It can be
constructed with all necessary parameters (name and description) and allows adding child commands
that may be executed as sub commands. Arbitrary command hierarchies can be created by nesting
parent commands into each other.

Example
+++++++

The following code implements a simple command named "test" that offers the two sub commands "get"
and "put" that allow read and write access to a contained value.

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
    {}

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

Whenever you need a small footprint command regarding RAM usage then you should use the
``GroupCommand`` as a base class for your command functionality. This class is the replacement for
the former ``::bios::CommandInterpreter`` base class.

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

The default implementations are heavily based on the usage of the class
``::util::command::CommandContext``.  Context objects of this type are used for providing
access to the command line arguments and the output stream for presenting textual results.

A command will need a defined list of arguments. The ``CommandContext`` class provides some useful
methods for scanning arguments such as identifiers, integer values, hex byte buffers or arbitrary
whitespace separated tokens. Additionally there are methods for checking certain conditions on the
arguments or whether there are pending arguments. See ``::util::command::CommandContext``
for a more detailed description.
