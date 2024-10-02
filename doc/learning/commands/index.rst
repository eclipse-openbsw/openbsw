.. _learning_commands:

Adding commands to the console
==============================

Previous: :ref:`learning_logging`

See :ref:`util_command` for info on the facilities available to implement commands on the console.

The simplest way to add your own commands is to follow the examples of those already implemented.
If you search the code for ``GroupCommand`` you will find many example console command implementations.

One such example is ``StatisticsCommand``. You could add your own ``DemoCommand`` following this example.
Add a new file ``executables/referenceApp/consoleCommands/include/demo/DemoCommand.h`` containing...

.. code-block:: cpp

    #ifndef GUARD_988E20A7_FCA4_4055_A82E_826FCE85E1C6
    #define GUARD_988E20A7_FCA4_4055_A82E_826FCE85E1C6

    #include "util/command/GroupCommand.h"

    namespace demo
    {

    class DemoCommand : public util::command::GroupCommand
    {
    public:
        DemoCommand() {}

    protected:
        DECLARE_COMMAND_GROUP_GET_INFO
        virtual void executeCommand(util::command::CommandContext& context, uint8_t idx);
    };

    } // namespace demo

    #endif /*GUARD_988E20A7_FCA4_4055_A82E_826FCE85E1C6*/

and add a new file ``executables/referenceApp/consoleCommands/src/demo/DemoCommand.cpp`` containing...

.. code-block:: cpp

    #include "demo/DemoCommand.h"

    namespace demo
    {

    DEFINE_COMMAND_GROUP_GET_INFO_BEGIN(DemoCommand, "demo", "Demo Commands")
    COMMAND_GROUP_COMMAND(1U, "hello", "Print hello")
    DEFINE_COMMAND_GROUP_GET_INFO_END;

    void DemoCommand::executeCommand(util::command::CommandContext& context, const uint8_t idx)
    {
        switch (idx)
        {
            case 1:
            {
                printf("Hello World");
                break;
            }
            default:
            {
                break;
            }
        }
    }

    } // namespace demo

Look at how ``StatisticsCommand`` is added to ``RuntimeSystem``
and to ``executables/referenceApp/consoleCommands/CMakeLists.txt``
and add ``DemoCommand`` in the same way.
If you build with these additions you should see your new command in the console...

.. code-block:: bash

    > help

    demo       - Demo Commands
      hello    - Print hello
    help       - Show all commands or specific help for a command given as parameter.
    lc         - lifecycle command
      reboot   - reboot the system
      poweroff - poweroff the system
      level    - switch to level
      udef     - forces an undefined instruction exception
      pabt     - forces a prefetch abort exception
      dabt     - forces a data abort exception
      assert   - forces an assert
    stats      - lifecycle statistics command
      cpu      - prints CPU statistics
      stack    - prints stack statistics
      all      - prints all statistics
    ok

    > demo hello

    Hello World ok

    >

As you add your own functionality to a project, being able to add commands to help develop and test is a valuable asset.

Next: :ref:`learning_can`
