..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

User Documentation
==================

CanSystem
---------

The ``CanSystem`` class provides functionality for managing CAN communication in the system. It
provides methods to initialize, run and shutdown the CAN system, as well as to handle received
CAN frames. It defines a configuration for a CAN device, specifying the SocketCAN interface
name and bus ID.

Public API
++++++++++

.. literalinclude:: ../../include/systems/CanSystem.h
         :start-after: [PUBLIC_API_START]
         :end-before: [PUBLIC_API_END]
         :language: c++
