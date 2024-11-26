User Documentation
==================

The brief description of the classes and header files of this module is provided below:

Adc
---
  - The class ``Adc`` represents an Analog-to-Digital Converter with configurable resolution
    and a maximum number of channels.
  - The class defines different resolution modes for an ADC.
  - It provides methods for initializing the ADC, enabling specific channels, starting and
    stopping the ADC and getting the value from a specific channel.

adcPhysicalName
---------------
  - This file defines physical names that correspond to different ADC channels.

adcResolution
-------------
  - The class ``AdcInResolution`` is designed to handle the conversion of raw ADC data to a
    specific resolution.

adcScale
--------
  - Scales the input value using the provided scaling parameters.

AnalogInputScaleImplementation
------------------------------
  - ``AnalogInputScaleImplementation`` class provides functionality for scaling analog input values.
  - The class has methods for initialization, getting a scaled value for a specific channel and
    calculating a scaled value based on a provided ADC value.