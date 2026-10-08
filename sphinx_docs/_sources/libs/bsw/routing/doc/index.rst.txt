..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

routing - PDU routing across channels
=====================================

Network routers, on the most basic level, take some address or identification information and use
it to distribute a packet/data to one, multiple or no destinations.
This module defines such a router, trying to get rid of as much meta/context information as
possible for abstraction and optimization reasons.

What are the most basic bits of information?
An input/output-channel (port), identification (ID) and the data which needs to be forwarded/routed.
Only channel and ID are used for the routing decision, the data must not be interpreted.
No information about the content should be leaked implicitly e.g. by trying to encode this
information into the ID, so as not to break the layer separation.

To be able to plug data into this generic router, we need a way to give context to this data, and to
connect it with buses: the channel adapter.
An adapter packs and unpacks the data for routing: unpacking the PDUs from the format circulating on
the buses, and packing it back up in the correct format on the other end.

The module ``routing`` contains this PDU router, configured using a binary description of the
routing table. The PDUs can be routed across CAN, FlexRay or Ethernet.

.. uml::
   :align: center

   Package routing {
     () IReader as ir1
     () IWriter as iw1

     [Router]
     [RxAdapter]
     [TxAdapter]
   }

   () IReader as ir2
   () IWriter as iw2
   [MemoryQueue] as mq1
   [MemoryQueue] as mq2

   [Router] -left-( ir1
   [Router] -right-( iw1
   [RxAdapter] -right- ir1
   [TxAdapter] -left- iw1
   [RxAdapter] -down-( ir2
   [TxAdapter] -down-( iw2

   mq1 -up- iw2
   mq2 -up- ir2


.. toctree::
   :maxdepth: 1

   interface
   integration
   design
