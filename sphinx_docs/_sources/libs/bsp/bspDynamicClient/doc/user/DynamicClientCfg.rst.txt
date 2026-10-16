..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _DynamicClientCfg:

DynamicClientCfg
================

The module ``DynamicClientCfg`` contains the structure ``dynamicClient`` which provides a flexible mechanism
for managing dynamic I/O channels and their associated clients at runtime.
It allows for dynamic addition, removal, and querying of clients and channels, providing a
convenient interface for handling dynamic I/O configurations in embedded systems.

Enumeration
-----------

.. sourceinclude:: include/io/DynamicClientCfg.h
   :start-after:  [Enum_start]
   :end-before: [Enum_end]
   :language: c++

Methods
-------

.. sourceinclude:: include/io/DynamicClientCfg.h
   :start-after:  METHOD_START
   :end-before: METHOD_END
   :language: c++