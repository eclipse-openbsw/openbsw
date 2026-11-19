..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Memory Pool Allocation
======================

This functionality provides fixed-size memory allocation for message
payloads using free-list pools with multi-pool fallback.

Static View
-----------

The following software units are involved in memory pool allocation:

* Memory Pool Management Unit: Manages multiple pools with different block sizes
   (see :doc:`../units/memory_pool_management` for its class diagram)
* Message Management Unit: Requests memory for message payloads

.. uml:: memory_pool_allocation_component.puml
   :caption: Memory pool allocation component view

Notes:

* ``Aggregator`` routes requests to the appropriate pool based on size.
* Falls back to larger pools if smaller ones are exhausted.
* Each pool uses a free-list with ``pNext_chunk`` pointer chaining for fast
  allocation.
* Each free chunk contains a pointer to the next free chunk (or ``nullptr``
  if last).
* Allocation and deallocation are constant-time, O(1).

Dynamic View
------------

The dynamic view illustrates memory allocation with pool selection and
fallback.

Allocate
++++++++

.. uml:: memory_pool_allocation_allocate_sequence.puml
   :caption: Memory pool allocation sequence

Deallocate
++++++++++

.. uml:: memory_pool_allocation_deallocate_sequence.puml
   :caption: Memory pool deallocation sequence
