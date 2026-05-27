..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _safeIo:

safeIo
======

``safeIo`` monitors the IO configuration registers and provides methods to enter and leave IO safe state.
If any unexpected change in the IO configuration is detected, it sends an event to the :ref:`safeSupervisor`.

.. tolerant-toctree::
    :maxdepth: 1

    user/index
