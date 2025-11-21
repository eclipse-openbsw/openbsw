.. _bspIo_Input:

bspIo_Input
===========

Configuration
-------------

  1. ``inputConfiguration.h``: Input pin configuration.

      - The `sfDigitalInputConfigurations` array contains `ioNumber`, `isInverted` and
        `debounceThreshold` for each input configuration in their respective order.

      - The list of input pin IDs present in the ``DigitalInputId`` enum are corresponding to the
        order of configurations in the `sfDigitalInputConfigurations` array.

      - An Input pin ID is used to refer to the input pin in the application code.


  2.  ``inputConfigurationStrings.h``: List of input pin names for the debugging console.
      - The List of input pin names present in the `inputConfigurationStrings` array corresponds to
      the order of the `DigitalInputId` enum.

- For the demo purposes, EVAL_DI_1 is configured as digital input pin which reads the status of the
  push button on the evaluation board.

Digital Input
-------------
The `DigitalInput` class in the ``inputManager/DigitalInput.h`` is responsible for the
initialization of the digital input pins based on the ``inputConfiguration.h`` and for providing
the an application interface for reading the digital input pin status.

Application Interface
+++++++++++++++++++++

To get the digital input value, use the following function:

.. code:: c++

    bsp::BspReturnCode get(DigitalInputId channel, bool& result);

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

:source:`/../../../../../libs/bsp/bspInputManager/include/inputManager/DigitalInput.h`