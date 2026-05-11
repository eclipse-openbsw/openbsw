..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Routing Integration
===================

Integration
-----------

For projects using FlexRay, CAN, and PDU transport, the ``Integration`` class provides
a ready-to-go implementation of the instantiation of the routing components.

All of the routing classes can ``load`` their configuration from a span of bytes, and the
``Integration`` class uses the ``blob`` module to access the relevant information.

Here is the structure of this class, with regards to the ``Routing`` components (note that
``CHANNELS = PDU_TRANSPORT_CHANNELS + CAN_CHANNELS + FLEXRAY_CHANNELS``):

.. uml::
   :align: center

    namespace io {
       interface IWriter {}
       interface IReader {}
       hide members
   }
   namespace routing {
        class Integration<PDU_TRANSPORT_CHANNELS, CAN_CHANNELS, FLEXRAY_CHANNELS> {}
        class Router<N> {}
        class RxAdapter {}
        class TxAdapter {}


        Integration "1" *-- "1" Router
        Integration "1" *-- "CHANNELS" RxAdapter
        Integration "1" *-- "CHANNELS" TxAdapter
        Integration --> blob.Blob : "uses"

        Router "1" --> "CHANNELS" io.IWriter
        Router "1" --> "CHANNELS" io.IReader

        RxAdapter --|> io.IReader
        TxAdapter --|> io.IWriter

        hide members
   }
   namespace blob{
        class Blob

        hide members
   }


PDU Transport Integration
-------------------------

The ``PduTransportIntegration`` class takes care of instantiating all the components related to the UDP-based PDU
transport communication, except for the sockets. The sockets are provided at initialization and their type has
to derive from ``AbstractDatagramSocket``. ``PduTransportIntegration`` separates frame creation from
socket transmission:

* ``PduTransportBufferedWriter`` batches outgoing PDU transport messages into frame-sized queue
  entries
* ``checkTransmissionTimeouts()`` evaluates whether buffered frames must be committed to TX queues because
  of configured timeouts
* ``sendUdpFrames()`` begins the actual UDP transmission by reading frames from the TX queue and sending
  them through sockets

As a result, data already written to TX queues may still have not been sent out. Data is written to the TX queue when
the next message would no longer fit into the current frame buffer or the configured transmission timeout expires.
The timeout starts with the first committed message in an empty frame buffer, not with allocation.

The structure of this class is the following:

.. uml::
   :align: center

    namespace udp {
       interface AbstractDatagramSocket {}
       hide members
   }
   namespace io {
       class MemoryQueueWriter {}
       class MemoryQueueReader {}

       MemoryQueueWriter --> MemoryQueue
       MemoryQueueReader --> MemoryQueue

       namespace udp {
           class Receiver{}
           class Sender{}
       }
       hide members
   }
   namespace routing {
        class PduTransportIntegration<PDU_TRANSPORT_CHANNELS> {}

        PduTransportIntegration -r-> blob.Blob : "uses"
        PduTransportIntegration -r-> PduTransportConfig : "uses"

        PduTransportIntegration "1" *-- "2*PDU_TRANSPORT_CHANNELS" io.MemoryQueue
        PduTransportIntegration "1" *-- "2*PDU_TRANSPORT_CHANNELS" io.MemoryQueueReader
        PduTransportIntegration "1" *-- "2*PDU_TRANSPORT_CHANNELS" io.MemoryQueueWriter
        PduTransportIntegration "1" *-- "PDU_TRANSPORT_CHANNELS" io.udp.Receiver
        PduTransportIntegration "1" *-- "PDU_TRANSPORT_CHANNELS" io.udp.Sender
        PduTransportIntegration "1" *-- "2*PDU_TRANSPORT_CHANNELS" udp.AbstractDatagramSocket

        hide members
   }
   namespace blob{
        class Blob
        hide members
   }

If we look at the way these components are interconnected and connected with the ``Integration`` class, we get:

.. uml::
   :align: center

   component Integration {
     () IReader as ir1
     () IWriter as iw1

     [Router]
     [RxAdapter]
     [TxAdapter]
   }

   component PduTransportIntegration {
     () IReader as ir4
     () IWriter as iw4

     [Receiver]
     [MemoryQueue] as mq1
     [MemoryQueue] as mq2
     [Sender]
     () AbstractDatagramSocket as isock
     () AbstractDatagramSocket as osock
   }

   () IReader as ir3
   () IWriter as iw3

   [UdpSocket] as iudp
   [UdpSocket] as oudp

   [Router] -left-( ir1
   [Router] -right-( iw1
   [RxAdapter] -right- ir1
   [TxAdapter] -left- iw1
   [RxAdapter] -down-( ir3
   [TxAdapter] -down-( iw3

   mq1 -up- ir3
   mq2 -up- iw3

   mq1 -left- iw4
   mq2 -right- ir4
   [Receiver] -right-( iw4
   [Sender] -left-( ir4

   [Receiver] -down-( isock
   [Sender] -down-( osock

   iudp -up- isock
   oudp -up- osock
