..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _bspCharInputOutput:

bspCharInputOutput
==================

Configuration
-------------

``CharIOSerialCfg.h`` provides configuration for the serial input/output.
It is used for handling serial communication particularly for logging purposes.

This configuration defines the following macros:

`CHARIOSERIAL_BUFFERSIZE` sets the size of the asynchronous buffer for logger outputs.

`SCI_LOGGERTIMEOUT` defines a timeout value for logger operations, specifying the maximum time
(in milliseconds) that a logger operation can wait before timing out.
