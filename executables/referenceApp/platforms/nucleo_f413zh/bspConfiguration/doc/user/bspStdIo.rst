..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _bspConfig_StdIo_F413ZH:

bspStdIo
========

Overview
--------

The standard I/O bridge implements the ``putByteToStdout`` and
``getByteFromStdin`` C-linkage functions required by the OpenBSW logging
framework, routing character-level I/O through the UART ``TERMINAL`` instance
configured in :ref:`bspConfig_Uart_F413ZH`.

``putByteToStdout(uint8_t byte)`` transmits one byte over USART3 TX (PD8),
blocking by polling until the UART TX register is free.

``getByteFromStdin()`` attempts to read one byte from USART3 RX (PD9) and is
non-blocking: if ``Uart::read()`` reports that no byte was received, it returns
``-1`` (POSIX ``getchar()`` convention); otherwise it returns the byte value
(0-255).

Both functions resolve the UART instance once into a ``static`` local
reference, avoiding a global constructor ordering dependency.
