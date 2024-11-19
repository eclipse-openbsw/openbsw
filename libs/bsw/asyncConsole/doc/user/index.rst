User Documentation
==================

To set up the *AsyncConsole* all commands must be wrapped in either an ``AsyncCommandWrapper`` or a ``SyncCommandWrapper``. See :ref:`util_command` for more information on creating a command.

.. important::
   References to commands registered in the console, along with their ``AsyncCommandWrapper`` or ``SyncCommandWrapper``, are stored in static memory.
   To avoid lifetime issues, it is recommended to allocate all commands and command wrappers in static memory.

console::AsyncCommandWrapper
----------------------------

The constructor of the ``AsyncCommandWrapper`` class registers the wrapped command within the console.
This includes making the command executable by its name and adding relevant information to the help command. The second argument of the constructor allows specifying the context in which the command will be executed. When a wrapped command is run, the *AsyncConsole* will wait until the command has completed in the given context. Only then will further interaction be possible.

console::SyncCommandWrapper
---------------------------

The ``SyncCommandWrapper`` is a synchronous version of the ``AsyncCommandWrapper``. It is recommended for commands that must always run in the same context as the console itself. This synchronous wrapper is also used internally to register the help command.

console::AsyncConsole
---------------------

To interact with the *AsyncConsole*, input lines must be provided through the
``AsyncConsole::onLineReceived`` method. Refer to the referenceApp implementation to see how the
:ref:`stdio_console_input` module is used to forward console input to the *AsyncConsole*.

Example - registering a command
-------------------------------

Given the :ref:`bsw command <util_command>` class :ref:`DemoCommand<learning_commands>`, registering this DemoCommand to the *AsyncConsole* can be done as follows:

.. sourceinclude:: test/gtest/src/console/DemoTest.cpp
   :language: c++
   :start-after: EXAMPLE_START AsyncCommandWrapper
   :end-before: EXAMPLE_END AsyncCommandWrapper
   :dedent: 4
