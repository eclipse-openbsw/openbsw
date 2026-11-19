..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _StaticBsp:

Static BSP
==========

Overview
--------
`StaticBsp` class is part of Board Support Package (BSP). StaticBsp is low level module, that need to
be initialized before main. it contains the BSP class instances which are not started in the
lifecycle (BspSystem) but are available even before the lifecycle starts ("static").
