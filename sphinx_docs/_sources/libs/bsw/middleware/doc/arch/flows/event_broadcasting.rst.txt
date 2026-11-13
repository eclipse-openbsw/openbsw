..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Event Broadcasting
==================

This functionality enables a skeleton to broadcast events to multiple proxy
subscribers across different clusters and within the same cluster.

Static View
-----------

The following software units are involved in event broadcasting:

* Transceiver Unit (Skeleton side): Manages event emission and subscriber tracking
  (see :doc:`../units/proxy_skeleton_base` for its class diagram)
* Transceiver Unit (Proxy side): Receives and processes broadcast events
  (see :doc:`../units/proxy_skeleton_base`)
* Message Management Unit: Allocates event messages with reference counting
* Cluster Connection Unit: Routes events to subscribed clusters
* Message Queue Unit: Delivers events to proxy cores

.. uml:: event_broadcasting_component.puml
   :caption: Event broadcasting component view

Notes:

* ``EventSender`` broadcasts to all registered cluster connections.
* Each cluster has its own queue.
* Externally allocated payload is reference-counted for multi-cluster
  delivery (``ref_count = num clusters``).
* The last cluster to process the event deallocates the payload.

Dynamic View
------------

The dynamic view illustrates event broadcasting from skeleton to multiple
proxies with reference counting.

.. uml:: event_broadcasting_sequence.puml
   :caption: Event broadcasting sequence
