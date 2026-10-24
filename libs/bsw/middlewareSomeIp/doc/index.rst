..
   *******************************************************************************
   Copyright (c) 2026 KPIT Technologies Ltd.

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _middlewareSomeIp:

middlewareSomeIp - SOME/IP binding
==================================

Overview
--------

``middlewareSomeIp`` carries middleware services over SOME/IP. Generated skeletons and
proxies keep talking to a middleware cluster connection; the connection's configuration
is a ``SomeIpConnection``, which moves each message to and from a ``cpp2someip`` stack
instead of a shared-memory queue. Neither the skeleton nor the proxy knows the other
side is SOME/IP.

A connection carries every service placed on its cluster pair. Each service, or each
instance of one, is a ``SomeIpServiceEndpoint`` added to it; the connection routes every
message to the endpoint of its service and instance. One endpoint serves one provided
instance and, optionally, one consumed instance:

* methods and field getters/setters of the provided instance arrive as SOME/IP requests
  and are answered with the skeleton's response,
* events and field notifications of the provided instance are sent to the SOME/IP
  subscribers, or to a multicast group,
* subscribing the proxy of the consumed instance starts SD FindService and Subscribe for
  that instance; its events are then delivered to the proxy,
* the proxy's method calls and field getters/setters are sent as SOME/IP requests to
  where SD found the consumed instance, and the responses answer the proxy.

Every request a proxy sends is answered: with the response, or with an error for a
timeout or a SOME/IP error. A request that cannot be sent at all, because the instance
is not found yet or every request channel is busy, fails at once.

This module has the following dependencies:

* :ref:`cpp2someip`
* the middleware
* etl

Wire encoding
-------------

The middleware moves C++ objects between clusters; SOME/IP needs a byte stream.
``SomeIpCodec<Tag>`` supplies the encoding of one payload type. It is keyed on a tag
rather than on the type, because one C++ type can be written differently on different
members, for example a byte vector with an 8-, 16- or 32-bit length prefix.

``SomeIpCodec.h`` holds one template per kind of datatype in ``middleware::someip::codec``:
fixed and dynamic arrays, two-dimensional arrays, fixed and dynamic strings with their
byte order mark, enumerations bounded by their highest enumerator, and unions. A service
specializes ``SomeIpCodec`` for its tags by deriving from these. Scalars are streamed
natively and need no tag.

The specializations need not be written by hand: the middleware code generator
(``libs/bsw/middleware/tools/cpp_generator``) derives them from a FLYNC model with
``--flync``, as ``<Service>Codecs.h`` beside the service's generated types.

A decoded value that violates the model, such as an enumerator out of range, a string
with the wrong byte order mark, or bytes left over after the payload, fails the parser,
so the stack answers ``E_MALFORMED_MESSAGE``.

Usage
-----

#. Construct one ``SomeIpConnection<MaxEndpoints, MaxTransceivers>`` per bridged cluster
   pair, with the source and target cluster ids of the connection it replaces.
#. Per service, construct a ``SomeIpServiceEndpoint<MaxMembers, MaxPending>`` with the
   stack, its event sender, network and RPC handler, the service id, and a
   ``SomeIpBridgeConfig``: versions, instance ids, port, protocol, SD timing, the
   multicast group for multicast events, and for the consumed instance the request
   timeout and first client id. Add it to the connection with ``add()``.
#. Bind every member with a ``PayloadSlot<T, Tag>`` per payload direction:
   ``bindMethod()`` for methods and field getters/setters, ``bindEvent()`` for events and
   field notifiers; for the consumed instance, ``bindConsumedMethod()`` and
   ``bindConsumedEvent()``.
#. Call ``offerService()`` on each endpoint. Eventgroups are offered by registering a
   ``someip::ProvidedService`` per eventgroup on the same stack.
#. Make the cluster connection use the ``SomeIpConnection`` before any skeleton or proxy
   is initialized, since they register with whichever configuration the connection holds
   at that point. The generated connection object is constexpr-referenced by the
   instances database and holds its configuration by reference, so this is done by
   re-constructing it in place.
#. Initialize the skeletons and proxies on the SOME/IP stack's task: a skeleton must be
   initialized on the task it sends from.

Two provided instances of one service need different ports: a SOME/IP request carries
no instance id, so the port is what tells them apart.

A consumer receives an event only on the port it subscribed from, its ``providerPort``,
so a multicast group should use that port. ``isConsumedInstanceAvailable()`` tells when
SD has found the consumed instance; until then a request to it fails at once.

Sample
------

The referenceApp carries three members of the FLYNC model's service over the bridge
when ``PLATFORM_SUPPORT_SOMEIP_BRIDGE_SAMPLE`` is on, which is the default for POSIX:
``echoUINT8``, the ``TestFieldUINT8`` field and the ``TestEventUINT8Multicast`` event. The
code is in ``executables/referenceApp/application/include/middleware``; how to run
it is in ``tools/someip/MiddlewareBridgeSample.md``.

Limitations
-----------

* One provided and at most one consumed instance per endpoint, and one subscribed
  eventgroup of the consumed instance; more instances take more endpoints.
* At most ``MaxPending`` requests to the consumed instance in flight: ``cpp2someip``
  allows one outstanding call per request channel, and an endpoint keeps ``MaxPending``
  channels, each with its own client id.
* Everything runs on the SOME/IP stack's task: the connection and its endpoints hold no
  locks.
* A skeleton that never answers a request holds its request slot.
* Initial events are not replayed to a new subscriber.
* E2E protection is not derived from the model; an E2E-protected member needs a
  hand-written codec.
