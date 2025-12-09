.. _bspConfig_AnalogInput:

bspAdc
======

Configuration
-------------

1. ``AnalogInput.h``: Register configuration for 12-bit ADC.
    - The enum inside `Adc12BitConfiguration` class defines the values for registers.

2. ``analogInputConfiguration.h``: ADC input channel configuration.
    - The adc input signal mapping to digital pin is present in the `ADChannelCfg` array.

    - The format inside the array is as follows:

    `<ADC Module>`, `<ADC Channel>`, `<Io_pin>`, `<conversion channel>`.

    - The list of ADC input signal IDs present in the `AnalogInputId` enum corresponds to the
      order of the `ADChannelCfg` array.

    - The value inside the enum will be used to select the channel in the application.

3. ``analogInputConfigurationString.h``: List of adc input signal names for the debugging console.
    - The order of the `adcChannelsNamingStrings` array corresponds to the `AnalogInputId` enum.

4. ``analogInputScaleConfiguration.h``: Scaling configuration applicable to adc signals.
    - The `analogInputScaleConfiguration` array contains the list of scaling functions available
      with configurable precision and resolution (Scaling Factor).

    - The order of configuarations for each channel corresponds to `AnalogInputId` enum.

Analog Input
------------

`AnalogInput` class in the ``AnalogInput.h`` is handling initialization, starting conversions
and the stopping of adc module based on the configuration provided in the
``analogInputConfiguration.h``.

Analog Input Scale
++++++++++++++++++

`AnalogInputScale` class in the ``AnalogInputScale.h`` is handling the scaling of raw ADC value
to the application parameter based on the configuration provided in the
``analogInputScaleConfiguration.h``.

Application Interface
+++++++++++++++++++++

To get a raw 12-bit value from ADC, use

.. code:: c++

    bsp::BspReturnCode bios::AnalogInput::get(uint16_t channel, uint16_t& value)

To get the scaled value, use

.. code:: c++

    bsp::BspReturnCode bios::AnalogInputScale::get(uint16_t channel, uint16_t& value)

Example
-------

.. code-block:: cpp

    #ifdef PLATFORM_SUPPORT_IO
    #include "bsp/adc/AnalogInputScale.h"
    #endif

    void DemoSystem::cyclic()
    {
    #ifdef PLATFORM_SUPPORT_IO
        uint32_t value;
        (void)AnalogInputScale::get(AnalogInput::AiEVAL_POTI_ADC, value);
    #endif
    }

This code snippet placed in the demo system could read the ADC value and store the scaled data in the
variable `value` in the context menu.

As per the demo, ADC channel is configured to read a value from internal potentiometer
(``ADC0 - Channel_28``) available in the evaluation board. The value is then scaled to
milli voltage (0-5000mV).
