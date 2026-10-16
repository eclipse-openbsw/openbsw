..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Core
====

The core software unit of the middleware provides the key algorithms needed for the communication between applications.

Middleware Message
------------------

Middleware is a service-oriented message passing system. The ``Message`` class is its fundamental unit of communication.
This class consists of a header and a payload.
The header has the following information:

* Service ID - identifies the service to which the message belongs.
* Member ID - identifies the specific member function of the service that is being invoked.
* Request ID - identifies the specific request being made.
* Instance ID - identifies the instance of the service (multiple instances may exist).
* Source cluster ID - identifies the cluster from which the message originated.
* Destination Cluster ID - identifies the cluster to which the message is being sent.
* Address ID - identifies the recipient of the message within the destination cluster.

The middleware services' nodes will be scattered across different clusters in the ECU.
This means that messages can travel between clusters. Each ``Message`` must carry information about its origin and destination.
Additionally, a message may have several possible recipients. Each recipient needs a unique identifier after system initialization.
To this end, the header contains the source cluster ID, target cluster ID, and address ID fields.

Finally, the payload contains the actual data being transmitted. The message
capacity is defined by ``Message::MAX_PAYLOAD_SIZE`` as
``Message::MAX_MESSAGE_SIZE - sizeof(Message::Header) - sizeof(uint32_t)``.
With the current 64-byte message layout, this capacity is 48 bytes.
Payloads that fit and are trivially copyable are stored in the message's
internal buffer. Larger or non-trivial payloads are copy-constructed into
middleware-managed external storage, and the message records an external
payload offset and the ``hasExternalPayload`` flag. ``Message`` also supports
an ``ErrorState`` payload for error responses.
