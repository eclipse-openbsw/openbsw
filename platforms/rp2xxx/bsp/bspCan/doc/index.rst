..
   *******************************************************************************
   Copyright (c) 2026 Roland Reichwein

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspCan
======

Overview
--------

The ``bspCan`` is a driver for the FlexCAN controller. The class
``FlexCANDevice`` contains the configuration, state, and behavior of the device.
It includes methods for initializing, starting, and stopping the device, as well
as transmitting and receiving data. The class ``FlexCANDevice`` also provides
methods for handling interrupts, managing transmit buffers, and querying device
status such as error counters and bus state.