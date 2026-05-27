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

This module provides two classes.

- *SafeIo* monitors configuration registers of safety relevant IOs.
- *SafeState* provides methods to enter and leave a safe state for safety relevant IOs.

Public API
----------

.. sourceinclude:: include/safeIo/SafeIo.h
    :start-after: PUBLIC_API_START
    :end-before: PUBLIC_API_END
    :language: c++
    :dedent: 4
    :caption:

.. sourceinclude:: include/safeIo/SafeState.h
    :start-after: PUBLIC_API_START
    :end-before: PUBLIC_API_END
    :language: c++
    :dedent: 4
    :caption:
