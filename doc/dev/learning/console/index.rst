.. _learning_console:

Using the console
=================

Previous: :ref:`learning_layout`

A console is provided in which you can enter commands and see log messages from the application.
On an embedded platform this is usually provided as a serial console over UART.
On the POSIX platform, it is provided in the standard I/O of the shell in which the application is executed.

If you press return, you should see that a prompt ``>`` appears, indicating commands may be entered.
If you enter the command ``help`` and press return a list of available commands is printed.

.. toctree::
   :hidden:

   cpuStatistics
   loggerConsole
   ioConsole
   adcConsole
   pwmConsole
   lcConsole

Available Console Commands
--------------------------

POSIX Platform
++++++++++++++

.. code-block:: bash

 > help
 can        - Can system.
   info     - print bus info
   send     - send frame: id data[8]
              [send 0x123 1 2 3 4 5 6 7 8] sends to CAN_0 Frame(CanId = 0x123)
 help       - Show all commands or specific help for a command given as parameter.
 lc         - lifecycle command
   reboot   - reboot the system
   poweroff - poweroff the system
   level    - switch to level
   udef     - forces an undefined instruction exception
   pabt     - forces a prefetch abort exception
   dabt     - forces a data abort exception
   assert   - forces an assert
 logger     - logger settings
   level    - get/set levels. Usage: logger level [<component>] [<new_level>]
 stats      - lifecycle statistics command
   cpu      - prints CPU statistics
   stack    - prints stack statistics
   all      - prints all statistics
 ok

S32K148EVB Platform
+++++++++++++++++++

.. code-block:: bash

 > help
 adc         - Adc Console
   all       - Print All Adc
   get       - startChannel ChannelNr [stopChannel ChannelNr]
   allx      - all scaled adc
 can         - Can system.
   info      - print bus info
   send      - send frame: id data[8]
               [send 0x123 1 2 3 4 5 6 7 8] sends to CAN_0 Frame(CanId = 0x123)
 help        - Show all commands or specific help for a command given as parameter.
 in          - inputs console
   all       - print all inputs, verbose output
   get       - get input value (param: <input-id>)
   allx      - all inputs
 lc          - lifecycle command
   reboot    - reboot the system
   poweroff  - poweroff the system
   level     - switch to level
   udef      - forces an undefined instruction exception
   pabt      - forces a prefetch abort exception
   dabt      - forces a data abort exception
   assert    - forces an assert
 logger      - logger settings
   level     - get/set levels. Usage: logger level [<component>] [<new_level>]
 out         - outputs console
   all       - print all outputs
   set       - set output value (param: <output-id> [0|1])
   get       - get output value (param: <output-id>)
 pwm         - Set PWM
   all       - print all channel numbers and names
   set       - channel[0-9,0xFF=all] duty[0=off|10000=100%]
   setPeriod - channel[0-9] period[microsec]s
 stats       - lifecycle statistics command
   cpu       - prints CPU statistics
   stack     - prints stack statistics
   all       - prints all statistics
 ok

In the above menu printed by ``help``, indented items are sub-commands of the unindent command above it.

Explore these commands, look at the documentation :ref:`util_command`
and search the code for occurrences of ``GroupCommand`` to improve your understanding.

Read more about individual commands from below:

.. csv-table::
   :widths: 30, 70
   :width: 100%

    :ref:`cpu_statistics`,"Statistics that are collected by the CPU and monitor in the console."
    :ref:`logger_console`,"The console logger helps the user to switch between the logger level."
    :ref:`digital_io`, "The ``out`` command controls the Digital Output Channel, while the ``in`` command reads the Digital Input Channel."
    :ref:`adc_console`, "The ``adc`` command reads the ADC channel value."
    :ref:`pwm_console`, "The ``pwm`` command sets the duty cycle for a PWM channel."
    :ref:`lifecycle_console`, "The ``lc`` command provides lifecycle commands like reboot, poweroff, etc."

Next: :ref:`learning_unit_tests`
