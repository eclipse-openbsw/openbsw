..
   *******************************************************************************
   Copyright (c) 2026 Roland Reichwein

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

threadXCoreConfiguration
========================


Module Overview
---------------
The module ``threadXCoreConfiguration`` provides configuration for ThreadX.

It supplies ``tx_user.h``, which overrides the defaults of the ThreadX port. The
priority count and the timer tick rate are derived from the async configuration,
so that ThreadX and the async framework agree on the number of tasks and on the
system tick interval.
