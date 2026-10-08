..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Components
==========

The middleware is organized into the following components, each grouping
one or more units with a coherent responsibility:

Communication Core Component (``communication_core_component``)
   Defines communication patterns used across clusters.
   This component isolates communication behavior so it can change without
   impacting storage or platform layers.

Communication Infrastructure Component (``communication_infrastructure_component``)
   Handles configuration and transport wiring for instance discovery and
   inter-cluster routing.
   This component isolates topology and deployment concerns.

Data Structures Component (``data_structures_component``)
   Provides reusable concurrent containers and memory management.
   This component centralizes low-level data handling to keep communication
   logic focused on behavior.

Interface Component (``interface_component``)
   Handles platform logging and OS abstraction interfaces.
   This component isolates platform variability and logging integration.

Cluster lifecycle and compile-time deployment rules (cluster initialization,
static resource allocation) are covered by the Communication Core and
Communication Infrastructure components.

