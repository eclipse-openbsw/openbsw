.. _bspIo_OutputPwm:

bspIo_OutputPwm
===============

Configuration
-------------

- ``outputPwmConfiguration.h``: List of static and dynamic PWM channels defined in
    the `outputPwm` enum .

    - The value inside the enum will be used to select the PWM channel in the application.


- ``outputPwmConfigurationStrings.h``: List of PWM channel names for the debugging console.

    - The order of the ``outputPwmConfigurationStrings`` array is corresponding to the `outputPwm`
        enum.


- ``PwmSupportConfiguration.hpp``: Contains the configuration for the PWM channel as follows:

    - `PWM Mode`, `interruptActive`, `dmaActive`, `icrst`, `IO pin Mapping`, `minimum` and
        `maximum` dutycycle.

- For demo purposes `EVAL_LED_RED_PWM`, `EVAL_LED_GREEN_PWM` and `EVAL_LED_BLUE_PWM` are configured
  as the PWM channels for controlling the brightness of red, green and blue internal LEDs available
  in the evaluation Board.

PwmSupport
----------

`PwmSupport` class defined in the ``PwmSupport.h`` is responsible for handling:

- `init`: Initializes and sets the dynamic client for all PWM channels.
- `start`: Starts all PWM channels.
- `stop`: Stops all PWM channels.
- `shutdown`: Stops and clears the dynamic client for all PWM channels.
- `setDuty`: Sets the duty cycle for the given PWM channel.
- `setPeriod`: Sets the period for the given PWM channel.

Application Interface
+++++++++++++++++++++

To set the duty cycle for a given PWM channel, use

.. code:: c++

   bsp::BspReturnCode setDuty(uint16_t chan, uint16_t duty, bool immediateUpdate);

Example
+++++++

.. code-block:: cpp

    #ifdef PLATFORM_SUPPORT_IO
    #include "outputPwm/OutputPwm.h"
    #endif

    void DemoSystem::cyclic()
    {
    #ifdef PLATFORM_SUPPORT_IO
        uint32_t value = 2500;  // 2500 is the sample value for 50% dutycycle

        OutputPwm::setDuty(OutputPwm::EVAL_LED_RED_PWM, value * 10000 / 5000);
        OutputPwm::setDuty(OutputPwm::EVAL_LED_GREEN_PWM, value * 10000 / 5000);
        OutputPwm::setDuty(OutputPwm::EVAL_LED_BLUE_PWM, value * 10000 / 5000);

    #endif
    }

This code snippet demonstrates how to set the duty cycle for the red, green and blue LEDs based
on the value provided by the application.
In demo application the value is updated by ADC based on the internal potentiometer available in
the evaluation board.