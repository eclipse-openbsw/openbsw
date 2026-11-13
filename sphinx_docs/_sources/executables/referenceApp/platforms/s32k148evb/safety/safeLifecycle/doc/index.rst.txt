..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _safeLifecycle:

safeLifecycle
=============

The ``safeLifecycle`` module serves as the central point for safety application and features. All
safety-related init, cyclic and shutdown operations are centralized within the SafetyManager,
making it easier to maintain a single safety object that can be connected to the lifecycle.
This simplifies code readability, reviews and supports easier optimization.

.. tolerant-toctree::

    user/index
