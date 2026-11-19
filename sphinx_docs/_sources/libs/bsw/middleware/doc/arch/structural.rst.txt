..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Structural View
===============

The middleware provides service-oriented interfaces for communication between
applications running in the same process, on different cores, or in separate
processes. Generated service code is split into application-facing proxies and
skeletons and middleware-owned cluster and queue infrastructure.

The system is divided into the following conceptual sub-components. The names
in this view describe architecture responsibilities, not necessarily one C++
class or one documentation page:

* service interface proxy
* service interface skeleton
* cluster
* queue

Service Interface Proxy
    It is composed of a core part ``proxy_base`` and a generated part
    ``generated proxy``. The generated proxy is created from an interface
    definition and sends requests through ``cluster_connection``.

Service Interface Skeleton
    It is composed of a core part ``skeleton_base`` and a generated part
    ``generated skeleton``. The generated skeleton receives requests through
    ``cluster_connection`` and dispatches them to the application.

Cluster
    A cluster owns the processing context for its service endpoints. It reads
    incoming messages from its queue, dispatches requests to skeletons, and
    routes responses and events to proxies.

Queue
    A queue provides FIFO transport between cluster connections. Queue and
    allocator configuration is generated from the deployment model.

.. note::

     The conceptual terms map to the current implementation as follows:

     * ``proxy_base`` and ``skeleton_base``: ``ProxyBase`` and ``SkeletonBase``
       in ``middleware/core``.
     * ``cluster_connection``: ``IClusterConnection`` and
       ``ClusterConnectionBase``.
     * ``queue`` and message transport: ``Message``,
       ``ClusterConnectionBase``, and the platform/shared-memory queue
       configuration.
     * message payload handling: ``MessagePayloadBuilder`` and the memory
       allocator types documented by the memory-pool unit.

     The units toctree documents the detailed class diagrams currently
     maintained for the proxy/skeleton base, request/response, and memory-pool
     abstractions. Cluster connection and queue behavior is documented through
     the component-flow diagrams because their concrete implementations are
     platform/deployment-specific.

.. uml:: middleware_structure.puml
    :caption: Middleware unit structure
