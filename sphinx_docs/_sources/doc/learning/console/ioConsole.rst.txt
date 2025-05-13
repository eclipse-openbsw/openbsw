.. _digital_io:

Digital Out
===========

The out console command is used to control the value of the Digital Output
Channel. The command takes DIO Channel as argument.

Out Command
-----------

The ``out`` command used to set the value of the Digital Output Channel.

-  **out**     - outputs console
    - all       - print all outputs
    - set       - set output value (param: <output-id> [0|1])
    - get       - get output value (param: <output-id>)

Below is the example usage:

.. code-block:: bash

  out get 0
  0 : eval_DO_1 -> 0
  ok
  669810: RefApp: CONSOLE: INFO: Received console command "out get 0"
  669812: RefApp: CONSOLE: INFO: Console command succeeded

.. code-block:: bash

  out set 0 1
  0 : eval_DO_1 -> ok
  ok
  763772: RefApp: CONSOLE: INFO: Received console command "out set 0 1"
  763776: RefApp: CONSOLE: INFO: Console command succeeded

Digital In
==========

The ``in`` console command is used to read the value of the Digital input Channel.
The command takes DIO Channel as argument.

In Command
----------

-  **in**     - inputs console
     - all       - print all inputs, verbose output
     - get       - get input value (param: <input-id>)
     - allx      - all inputs

Below is the example usage:

.. code-block:: bash

  in get 0
  dInput 0 -> 0
  ok
  302319: RefApp: CONSOLE: INFO: Received console command "in get 0"
  302321: RefApp: CONSOLE: INFO: Console command succeeded

Next: :ref:`adc_console`