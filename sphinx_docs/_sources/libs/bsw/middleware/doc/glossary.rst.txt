..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

:orphan:

Glossary
========

.. Please order the glossary entries alphabetically!

.. glossary::

   attached core
     A core that does not construct middleware shared runtime objects itself.
     It attaches to already initialized shared memory and binds to the existing
     ``MemoryLayout`` instance.

   Application Cluster
   Cluster
   middleware cluster
     A middleware cluster is a logical grouping of middleware endpoints running
     in one task context. Proxies and skeletons assigned to the same cluster
     share one incoming queue and are processed together by a cluster processing
     entrypoint.

   cluster connection
     A cluster connection is the generated middleware object that routes messages
     from one cluster to another. It dispatches incoming messages to proxies and
     skeletons and provides the write path to the destination queue.

   Core
     A CPU execution context inside one ECU instance. In multi-core setups, each
     core can host its own middleware cluster(s) and lifecycle loop.

   ECU
   Electronic Control Unit
     An ECU is a vehicle control unit that runs one or more software components
     and may communicate with other ECUs and other local cores via middleware.

   global deployment
     The model input that defines where middleware entities are deployed, for
     example queue names, ECU/core assignment, cluster IDs, and service-provider
     placement.

   integration and initialization
     The practical setup steps required to make middleware communication work
     in an ECU:

     * Wire generated code into the build.
     * Create or attach shared memory.
     * Initialize allocators and queues.
     * Initialize cluster connections.
     * Run cluster processing cyclically.

   Message
     The middleware transport unit exchanged between clusters through queues.
     A message typically carries header metadata (such as source cluster and
     service/member identifiers) plus payload bytes.

   Modeling the interfaces
     The authoring step where communication APIs are described in IDL/model
     files. In this user guide it focuses on service interfaces.

   owner core
     The single core responsible for constructing
     ``middleware::shm::MemoryLayout`` exactly once in shared memory during
     startup before other participating cores attach.

   payload
     The application data carried by a message, excluding protocol metadata
     such as headers, routing information, or control fields.

   Platform cluster entrypoints
     Generated functions for the platform cluster lifecycle, typically including
     initialize, process, and timeout-update entrypoints. They are called by the
     application lifecycle to drive middleware routing and timeout handling.

   Provider
   Consumer
     A provider (server, skeleton side) offers a service interface. A consumer
     (client, proxy side) uses that interface by sending requests, receiving
     responses, and subscribing to updates.

   queue
   Queues
     Middleware FIFO buffers used for inter-cluster message transport. Each
     cluster typically has an incoming queue that is polled and dispatched during
     cluster processing.

   Service
   Services
     A service is a middleware API contract containing methods, attributes, and
     events/broadcasts. Services are implemented by providers and used by
     consumers.

   service interface
   Service interfaces
     Interface definitions used for service-oriented communication. They describe
     typed methods, attributes, and events independent of the low-level transport
     mechanism.

   shared memory region
   SHM
     The shared memory area used by middleware runtime objects such as queues,
     allocator instances, and synchronization primitives. One core usually
     initializes this region and other cores attach to it.

   Software Component
   SwC
     A deployable software unit that owns application logic. SwCs use middleware
     through generated proxies/skeletons and wrapper code.

   startup barrier
     A platform synchronization point used to signal that owner-core shared
     memory construction is complete and attached cores may safely bind to the
     shared ``MemoryLayout`` instance.

   task
   RTOS task
     A schedulable execution context provided by the operating system. Middleware
     uses the current task identity to associate application clusters with their
     execution context. This may be an RTOS task handle, a POSIX thread ID, or a
     fixed value on a bare-metal single-task system.
