..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _bspConfig_Can:

bspCan
======

Configuration
-------------
The ``canConfiguration.h`` contains the ``FlexCANDevice::Config`` configuration structure which has
base address, baudrate, clock setup register, number of standard receive buffers, number of
extended receive buffers, etc as the parameters.

- The Baudrate value is taken from the enum values defined in the `ICanTransceiver` class.
- The `clockSetupRegister` value is calculated for the timing parameters based on:

    - `PRESDIV`: Prescaler division factor
    - `PSEG1`: Phase segment 1
    - `PSEG2`: Phase segment 2,
    - `PROPSEG` : Propagation segment
