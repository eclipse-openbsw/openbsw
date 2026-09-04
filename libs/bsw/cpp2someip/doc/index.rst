..
   *******************************************************************************
   Copyright (c) 2026 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _cpp2someip:

cpp2someip - SOME/IP Abstractions
=================================

Overview
--------

``cpp2someip`` provides an implementation of the SOME/IP stack.

It covers the following features:

* SOME/IP RPC messages via UDP and TCP
* SOME/IP service discovery (SD)
* SOME/IP TP
* Serialization and deserialization of SOME/IP messages

This module has the following dependencies:

* :ref:`async`
* :ref:`cpp2ethernet`
* etl

Configuration
-------------

This module offers two implementations of the SOME/IP stack: The ``RpcSomeIpStack`` and
the ``SdSomeIpStack``. The former can be used if no service discovery is required, while
the latter is used in a more dynamic environment. In the following description the ``SdSomeIpStack``
is used as an example, because it can be seen a superset of the ``RpcSomeIpStack``.

Stack configuration
+++++++++++++++++++

Configuration of the stack is done by setting appropriate template parameters. In the
case of the ``SdSomeIpStack`` the following template parameters are available:

- UdpSocketType (e.g. ``LwipDatagramSocket``)
- NumUdpRpcSockets (amount of Rpc Sockets needed)
- TcpServerSocketType (e.g. ``LwipServerSocket``)
- NumTcpServerSockets, (amount of Tcp Server Sockets needed)
- TcpSocketType (e.g. ``LwipSocket``)
- NumTcpRpcSockets (amount of Tcp Client Sockets needed)
- BufferSize (size of buffer used by socket to store incoming data)
- NumTpStreams (amount of streams used for SOME/IP TP)
- SdEndpointOption (use additional endpoint for service discovery)
- InternalTcpReassembleBufferSize (size of buffer used to reassemble incoming TCP data)
- NumRemoteEndpoints (amount of remote endpoints that can be connected to)
- NumRemoteServices (amount of remote services that can be connected to)
- NumRemoteSubscriptions (amount of subscriptions from remote endpoints)
- NumLocalServices (amount of local provided services)
- NumLocalQueries (amount of local consumed services)
- NumSubscriptionEndpoints (maximum amount of subscriptions per service)
- NumEventBuffers (amount of buffers for sending event messages)
- NumMulticastReceptions (amount of multicast receptions for SOME/IP notifications)

A typical configuration of the stack could look like this:

.. code-block:: cpp

    ::someip::declare::SdSomeIpStack<
        ::udp::LwipDatagramSocket,
        NUM_UDP_SOCKETS,
        ::tcp::LwipServerSocket,
        NUM_TCP_SERVERS,
        ::tcp::LwipSocket,
        NUM_TCP_SOCKETS,
        BUFFER_SIZE,
        0U,
        false,
        REASSEMBLE_BUFFER_SIZE,
        NUM_REMOTE_ENDPOINTS,
        NUM_REMOTE_SERVICES,
        NUM_REMOTE_SUBSCRIPTIONS,
        NUM_LOCAL_SERVICES,
        NUM_LOCAL_QUERIES,
        NUM_SUBSCRIPTION_ENDPOINTS>
        _stack;

Public API
----------

Startup
+++++++

Before actually calling ``SdSomeIpStack::start()`` the used ports and the overall stack have to be
initialized. This is done by calling ``SomeIpStack::initSdPort``, ``SomeIpStack::initUdpPort`` and/or
``SomeIpStack::initTcpPort`` (if necessary) and finally ``SdSomeIpStack::init()``. After that, if
notifications will be received by multicast the respective multicast endpoints can be set by calling
``RpcReceiver::requestMulticastReception`` and finally the stack can be started by calling
``SdSomeIpStack::start()``.

Runtime
+++++++

During runtime provided services and consumed services can be registered at the stack as follows:

- ``SomeIpStack::registerProvidedService`` (once for the service instance and once for each provided eventgroup)
- ``SomeIpStack::unregisterProvidedService`` (once for the service instance and once for each provided eventgroup)
- ``SomeIpStack::addEventListener`` (once per consumed service)
- ``SomeIpStack::registerServiceQuery`` (once for the service instance and once for each consumed eventgroup)
- ``SomeIpStack::unregisterServiceQuery`` (once for the service instance and once for each consumed eventgroup)

There are several reasons why it might be necessary to stop the stack during runtime. One of those could
be an ethernet link down. In that case the stack can be stopped by calling ``SomeIpStack::stop()``. After the link
is up again the stack can be restarted by calling ``SomeIpStack::start()`` again. If multicast notifications
are used they should also be stopped (``RpcReceiver::cancelMulticastReception``) and re-added after the link is
up again (``RpcReceiver::requestMulticastReception``) and before the stack is started again to ensure that
no notifications are missed.

Shutdown
++++++++

During shutdown a call of ``SomeIpSystem::shutdown`` will stop the stack and free all resources. This includes
sending stopOffer and stopsubscribe messages.
