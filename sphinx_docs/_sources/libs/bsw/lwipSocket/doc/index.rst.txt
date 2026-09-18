..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

lwipSocket - LwIP v2 Socket Abstraction
=======================================

Overview
--------

Module providing cpp2ethernet compatible implementations using lwip.

Configuration
-------------
This configuration of this module is available in `lwipopts` file located in
`executables/referenceApp/lwipConfiguration/include/lwipopts.h`

It includes the following options:
- Protocol support
- Memory and Buffer management
- TCP/IP protocol parameters
- Debugging and Logging options

To enable Logging for LWIP, you can set the necessary options under
`LWIP_DEBUG` section in `lwipopts.h` file. For example: NETIF_DEBUG, SOCKETS_DEBUG.
