..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Service Initialization
======================

This functionality enables proxies and skeletons to initialize themselves
from the instances database, establishing communication channels.

Static View
-----------

The following software units are involved in service initialization:

* Instance Database Unit: Provides static instance configuration lookup
* Transceiver Unit (Proxy side): Initializes proxy connections
   (see :doc:`../units/proxy_skeleton_base` for its class diagram)
* Transceiver Unit (Skeleton side): Initializes skeleton connections
   (see :doc:`../units/proxy_skeleton_base`)
* Cluster Connection Unit: Establishes communication channels

.. uml:: service_initialization_component.puml
   :caption: Service initialization component view

Notes:

* The instance database is compile-time generated from deployment configuration.
* Each service instance maps to specific cluster connections.
* Both proxies and skeletons can have multiple cluster connections based on
  deployment topology.
* Initialization validates instance IDs and returns error codes on failure.

Dynamic View
------------

The dynamic view illustrates proxy and skeleton initialization sequences
with error handling.

Proxy Initialization
++++++++++++++++++++

.. uml:: service_initialization_proxy_sequence.puml
   :caption: Service initialization proxy sequence

Skeleton Initialization
+++++++++++++++++++++++

.. uml:: service_initialization_skeleton_sequence.puml
   :caption: Service initialization skeleton sequence
