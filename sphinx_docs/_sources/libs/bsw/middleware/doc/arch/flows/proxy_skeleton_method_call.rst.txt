..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Proxy to Skeleton Method Call
=============================

This functionality enables a proxy in one cluster to invoke methods on a
skeleton located in a different cluster, with asynchronous request-response
handling. The same logic applies across different processes and/or threads.
The middleware supports both request-response methods (with return values)
and fire-and-forget methods (where no response is expected).

Static View
-----------

The following software units are involved in proxy-to-skeleton method calls:

* Transceiver Unit (Proxy side): Initiates requests and manages response callbacks
  (see :doc:`../units/proxy_skeleton_base` for its class diagram)
* Transceiver Unit (Skeleton side): Receives and processes requests
  (see :doc:`../units/proxy_skeleton_base`)
* Request/Response Unit: Manages asynchronous request lifecycle, response routing and timeouts (if enabled)
  (see :doc:`../units/request_response` for its class diagram)
* Message Management Unit: Allocates request/response messages and copy-constructs their payloads
* Cluster Connection Unit: Routes messages between clusters
* Message Queue Unit: Stores messages for inter-cluster transport

.. uml:: proxy_skeleton_method_call_component.puml
   :caption: Proxy to skeleton method call component view

Notes:

* Message flow is asynchronous through a shared memory queue.
* ``ProxyMethod`` manages pre-allocated future slots and invokes callbacks.
* A ``uint16_t`` request id is returned to the application for cancellation.

Dynamic View
------------

The dynamic view illustrates the runtime interactions for a method call with
an asynchronous response.

.. uml:: proxy_skeleton_method_call_sequence.puml
   :caption: Proxy to skeleton method call sequence

Notes:

* Depending on its size, the payload is copy-constructed either into the
  message's internal buffer or into an external allocation.
* Fire-and-forget methods follow the same flow but omit the response path: no
  ``Future`` is used, no response message is sent, and the skeleton does not
  call ``respond()``. The method is marked with
  ``call_semantic: FIRE_AND_FORGET`` in the service interface description.
