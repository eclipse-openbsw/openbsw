.. _bspIo_Output:

bspIo_Output
============

Configuration
-------------

- ``OutputConfiguration.h``: Output pin configuration.

  - The `sfOutputConfigurations` array contains `ioNumber`, `isInverted` and `debounceThreshold`
    for each output configuration in the respective order.

  - The list of output pin IDs present in `OutputId` enum are corresponding to the order of
    configurations in the `sfOutputConfigurations` array.

  - An output pin ID is used to refer to the output pin in the application code.


- ``OutputConfigurationStrings.h``: List of output pin names for the debugging console.

    - List of output pin names present in the `OutputConfigurationStrings` array are corresponding
      to the order of the `OutputId` enum.

- For demo purposes, EVAL_DI_1 is configured as digital output pin which reads the status of the
  push button on the evaluation board.

Output
------
The `Output` class in the ``outputManager/Output.h`` is responsible for the initialization of
the output pins based on the ``OutputConfiguration.h`` and for providing an application interface
for reading the digital output pin status.

Application interface
+++++++++++++++++++++

To get the output value, use the following function:

.. code:: c++

    static bsp::BspReturnCode set(OutputId chan, uint8_t val, bool latch = true);

Example
-------

.. code-block:: cpp

  void DemoSystem::cyclic()
  {
  #ifdef PLATFORM_SUPPORT_IO

      bool buttonStatus = false;

      // This is to read a digital input.
      DigitalInput::get(DigitalInput::EVAL_SW3, buttonStatus);
      // This is to set the output.
      Output::set(Output::EVAL_LED_RED, buttonStatus ? 1 : 0);

  #endif
  }

References
----------

:source:`/../../../../../libs/bsp/bspOutputManager/include/outputManager/Output.h`