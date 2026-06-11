..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _main:

main File
=========

This module is used to perform the necessary functionalities required to initialize the board
including configuration of phase locked loop, enabling cache and setting up ISR's priority.

``StaticBsp`` class is also instantiated for controlling BSP modules like ADC, PWM and CAN.

The watchdog mechanism used here is an example and should be adapted depending on the safety
concept of the real project.