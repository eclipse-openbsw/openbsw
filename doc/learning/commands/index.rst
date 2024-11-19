.. _learning_commands:

Adding commands to the console
==============================

Previous: :ref:`learning_logging`

See :ref:`util_command` for info on the facilities available to implement commands on the console.

The simplest way to add your own commands is to follow the examples of those already implemented.
If you search the code for ``GroupCommand`` you will find many example console command implementations.

One such example is ``StatisticsCommand``. You could add your own ``DemoCommand`` following this example.
Add a new file ``executables/referenceApp/consoleCommands/include/demo/DemoCommand.h`` containing...

.. sourceinclude:: ../../../libs/bsw/asyncConsole/test/gtest/include/demo/DemoCommand.h
    :language: c++
    :start-after: BEGIN DEMOCOMMAND
    :end-before: END DEMOCOMMAND

and add a new file ``executables/referenceApp/consoleCommands/src/demo/DemoCommand.cpp`` containing...

.. sourceinclude:: ../../../libs/bsw/asyncConsole/test/gtest/src/demo/DemoCommand.cpp
    :language: c++

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
