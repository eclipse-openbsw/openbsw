.. _main:

main File
=========

This module is used to perform the necessary functionalities required to initialize the board
including configuration of phase locked loop, enabling cache and setting up ISR's priority.

``StaticBsp`` class is also instantiated for controlling BSP modules like ADC, PWM and CAN.

The watchdog mechanism used here is an example and should be adapted depending on the safety
concept of the real project.