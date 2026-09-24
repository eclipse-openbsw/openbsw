..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Bus Identification Traits
=========================

Overview
--------

The module provides ``::common::busid::BusIdTraits`` class for bus identification.
The only trait supported now is the name of the bus.

Example
-------

.. code-block:: cpp

    auto busID = busid::CAN_0; // busID is a CAN bus
    printf("busID %d : %s  -> ...\n", busID, ::common::busid::BusIdTraits::getName(busID));
