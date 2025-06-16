.. _bspconfig_s32k148evb:

bspConfiguration - S32k148EVB
=============================

Overview
--------

The ``bspConfiguration`` module contains configuration related to hardware and board connections
specific to S32K148 evaluation board and parameters that are specific to the platform.
The logic of the drivers is separated from their configuration. This approach allows users
to customize their  project according to their requirements without impacting the driver code.
Configuration files are organized separately by modules.

Below are the modules available for configuration:

- ``bspAdc`` module provides an interface to read analog signals.
- ``bspCan`` module provides an interface for configuring and managing a `FlexCANDevice`.
- ``bspCharInputOutput`` module provides configurable settings for a logger system including
  buffer size, timeout values and debug mode.
- ``bspClock`` module contains macros for configuring clock settings in a microcontroller system.
  It defines constants related to crystal speed, CPU speed, clock dividers, and clock sources.
- ``bspIo_input`` module contains configuration related to input pins.
- ``bspIo_output`` module contains configuration related to output pins.
- ``bspPwm`` module provides an interface for configuring and managing a PWM device.
- ``bspIo`` module contains configuration related to input/output pins.
- ``bspSci`` module contains the configuration of the serial communication interface.
- ``bspTimer`` module contains the configuration of Flex Timer (FTM) register values.

.. toctree::
    :hidden:

    user/index
