..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Memory Pool Management
======================

This unit manages the fixed-size pools used for externally allocated middleware
payloads.

It provides allocation routing, pool aggregation, and usage metrics for the
memory-backed payload path.

Class Diagram
-------------

.. uml:: memory_pool_management.puml
