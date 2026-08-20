..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Service Interface
=================

Overview
--------

The ``middleware`` framework enables communication between processes running on the same core and  on a different core.
For service-based communication, generated code provides three integration points:

* ``<ServiceName>Proxy.h`` for clients (service consumers).
* ``<ServiceName>Skeleton.h`` for providers (service servers).
* ``<ServiceName>Common.h`` with shared data types and generated identifiers.

For unit and component tests, generation can additionally provide:

* ``<ServiceName>ProxyMock.h`` with a GoogleMock-based proxy replacement.
* ``<ServiceName>SkeletonMock.h`` with a GoogleMock-based skeleton replacement.

This chapter explains recommended middleware usage for application code that uses proxy and skeleton APIs.

Communication Patterns
----------------------

Middleware supports three communication patterns. Their exact API details are generated from the service model.

.. list-table:: Middleware communication patterns
   :header-rows: 1
   :align: center

   * - **Communication pattern**
     - **SOME/IP equivalent**
     - **Description**
   * - **methods**
     - method
     - The client sends a request and the server returns a response.
       ``FireAndForget`` methods are one-way requests without a response.
   * - **broadcasts**
     - events
     - The server sends an event to all subscribed clients.
       The server does not get any feedback about message reception.
   * - **attributes**
     - fields
     - The server stores an attribute value.
       Clients can read and/or write it based on model configuration.
       Attribute updates can also be sent as notifications to subscribed clients.

Key Terminology
---------------

* **Cluster**: middleware routes messages between application instances based on their configured :term:`Application Cluster`.
  Proxies and skeletons are initialized with generated instance identifiers plus cluster context.
* **Connected**: for static service mapping, a proxy or skeleton is considered connected when
  ``init(...)`` returns a successful registration result (normally
  ``::middleware::core::HRESULT::Ok``; an already-registered result is also
  treated as initialized by the generated proxy).
  If ``init(...)`` fails, do not call communication APIs.
* **Subscribed**: a client subscribes to a broadcast or attribute updates by registering a receive handler on the generated proxy event/attribute object.
* **Method**: request/response RPC from proxy to skeleton.
* **FireAndForget**: one-way method from proxy to skeleton with no response callback.
* **Event (broadcast)**: push-style update from skeleton to all subscribed proxies.
* **Attribute**: stateful value on the skeleton side with generated getter/setter and optional update notifications.

Compile-Check and Runnable Integration Strategy
-----------------------------------------------

This guide uses snippets from compile-checked sources under ``libs/bsw/middleware/doc/examples``.
Code examples demonstrating middleware API usage are available in the examples directory.

For runtime validation, integrate the same generated API calls in a component runnable and execute them in your target-specific integration tests.

Proxy Use Cases (Client Application)
------------------------------------

Instantiation and Initialization
--------------------------------

Proxy classes provide two lifecycle methods:

* ``init`` initializes middleware state and registers handlers.
* ``deInit`` unregisters the proxy from its cluster connection and clears
  pending method and attribute-getter futures, including their callbacks.
  It also clears registered event and attribute receive handlers through the
  proxy's internal notification-handler cleanup.

``init`` returns ``::middleware::core::HRESULT``.
Continue if the return value is a successful registration result, normally
``::middleware::core::HRESULT::Ok`` or
``::middleware::core::HRESULT::InstanceAlreadyRegistered``.

Proxy ``init`` requires:

* ``InstanceId``: target server instance.
* ``ClusterId``: source application cluster of the client.

Code Example for Client Side
++++++++++++++++++++++++++++

The following example shows proxy startup and shutdown.

.. literalinclude:: ../examples/release/proxy_app.h
   :language: cpp
   :start-after: [service-proxy-wrapper-start]
   :end-before: [service-proxy-wrapper-end]

Triggering and Receiving Method Calls as a Proxy
------------------------------------------------

Relevant proxy-side behavior:

* Generated method calls return ``::etl::expected<uint16_t, ::middleware::core::HRESULT>``.
  On success, the value is the request identifier.
  On failure, the error code explains why the request was rejected immediately.
* Request/response methods require a callback with signature ``etl::expected<etl::reference_wrapper<MethodOutputType const>, ::middleware::core::Future::State>``, also exposed as the generated ``<MethodName>Result`` alias.
  The callback receives either a const reference to the payload data or an asynchronous failure state.
* ``FireAndForget`` methods do not have response callbacks.

The example below covers asynchronous and ``FireAndForget`` method calls:

.. literalinclude:: ../examples/release/proxy_app.h
   :language: cpp
   :start-after: [service-proxy-method-start]
   :end-before: [service-proxy-method-end]

.. literalinclude:: ../examples/release/proxy_app.h
   :language: cpp
   :start-after: [service-proxy-fire-and-forget-start]
   :end-before: [service-proxy-fire-and-forget-end]

Testing Method Requests as a Proxy
++++++++++++++++++++++++++++++++++

Generated proxy mocks let a client test verify the request arguments, immediate
request identifier, and later response callback without a live transport.


.. literalinclude:: ../examples/test/proxy_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [proxy-async-method-test-start]
  :end-before: [proxy-async-method-test-end]

.. literalinclude:: ../examples/test/proxy_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [proxy-fire-and-forget-test-start]
  :end-before: [proxy-fire-and-forget-test-end]

Subscribing and Receiving Events as a Proxy
-------------------------------------------

For proxy usage, event subscription means registering a receive handler on the generated event object.
After registration, each received event triggers the callback.

* ``setReceiveHandler(...)`` for event notifications.

.. literalinclude:: ../examples/release/proxy_app.h
   :language: cpp
   :start-after: [service-proxy-method-subscription-start]
   :end-before: [service-proxy-method-subscription-end]

.. literalinclude:: ../examples/release/proxy_app.h
   :language: cpp
   :start-after: [service-proxy-broadcast-callback-start]
   :end-before: [service-proxy-broadcast-callback-end]

Testing Event Subscriptions as a Proxy
++++++++++++++++++++++++++++++++++++++

The proxy mock captures the registered handler so the test can simulate an
incoming event and verify that the application receives its payload.

.. literalinclude:: ../examples/test/proxy_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [proxy-event-receive-test-start]
  :end-before: [proxy-event-receive-test-end]

Reading and Writing Attributes as a Proxy
-----------------------------------------

Generated proxy attributes usually provide:

* ``get(callback)`` for asynchronous reads.

.. literalinclude:: ../examples/release/proxy_app.h
   :language: cpp
   :start-after: [service-proxy-attribute-read-start]
   :end-before: [service-proxy-attribute-read-end]

* ``set(value)`` for writes, if the model enables writing.

.. literalinclude:: ../examples/release/proxy_app.h
   :language: cpp
   :start-after: [service-proxy-attribute-write-start]
   :end-before: [service-proxy-attribute-write-end]

* ``setReceiveHandler(...)`` for update notifications.
  The application must explicitly send an attribute notification; changing
  the local value alone does not notify subscribers.
* ``unsetReceiveHandler()`` to remove an event or attribute receive handler.

.. literalinclude:: ../examples/release/proxy_app.h
   :language: cpp
   :start-after: [service-attribute-subscription-start]
   :end-before: [service-attribute-subscription-end]

.. literalinclude:: ../examples/release/proxy_app.h
   :language: cpp
   :start-after: [service-proxy-attribute-subscription-callback-start]
   :end-before: [service-proxy-attribute-subscription-callback-end]

Testing Attribute Getters as a Proxy
++++++++++++++++++++++++++++++++++++

The getter test captures the generated callback, checks the request identifier,
and then simulates the asynchronous attribute result.

.. literalinclude:: ../examples/test/proxy_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [proxy-attribute-get-test-start]
  :end-before: [proxy-attribute-get-test-end]

Testing Attribute Setters as a Proxy
++++++++++++++++++++++++++++++++++++

The setter test verifies that the value and immediate request result are
forwarded through the generated proxy mock.

.. literalinclude:: ../examples/test/proxy_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [proxy-attribute-set-test-start]
  :end-before: [proxy-attribute-set-test-end]

Testing Attribute Notifications as a Proxy
++++++++++++++++++++++++++++++++++++++++++

The notification test captures the registered handler, triggers it with a
simulated value, and checks the application state.

.. literalinclude:: ../examples/test/proxy_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [proxy-attribute-receive-test-start]
  :end-before: [proxy-attribute-receive-test-end]

Skeleton Use Cases (Server Application)
---------------------------------------

Code Example for Server Side
++++++++++++++++++++++++++++

Skeleton classes are server-side adapters.
The application derives from the skeleton class, implements generated virtual methods, and answers method requests.

Skeleton ``init`` receives only ``InstanceId`` (the server instance provided by this application).

The following example shows skeleton startup and shutdown.

.. literalinclude:: ../examples/release/skeleton_app.h
   :language: cpp
   :start-after: [service-skeleton-wrapper-start]
   :end-before: [service-skeleton-wrapper-end]

For skeleton-side methods:

* Each generated method is a virtual function to implement in the derived application class.
* Request/response methods receive input arguments plus ``SkeletonResponseInfo``.
* ``respond(...)`` must be called exactly once for request/response methods (immediately or later).
* For deferred responses, store ``SkeletonResponseInfo`` and respond in a later cycle.
* If no response can be sent, call the generated ``cancel...Response(...)`` API to release the pending request.


Handling Method Requests as a Skeleton
--------------------------------------

* Asynchronous request/response methods.

.. literalinclude:: ../examples/release/skeleton_app.h
   :language: cpp
   :start-after: [service-skeleton-method-start]
   :end-before: [service-skeleton-method-end]

.. literalinclude:: ../examples/release/skeleton_app.h
   :language: cpp
   :start-after: [service-skeleton-execute-start]
   :end-before: [service-skeleton-execute-end]

* Fire-and-Forget methods.

.. literalinclude:: ../examples/release/skeleton_app.h
   :language: cpp
   :start-after: [service-skeleton-fire-and-forget-method-start]
   :end-before: [service-skeleton-fire-and-forget-method-end]

Testing Method Requests as a Skeleton
+++++++++++++++++++++++++++++++++++++

The skeleton mock verifies initialization and forwarding of request/response
and fire-and-forget calls from the derived application class.

.. literalinclude:: ../examples/test/skeleton_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [skeleton-async-method-test-start]
  :end-before: [skeleton-async-method-test-end]

.. literalinclude:: ../examples/test/skeleton_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [skeleton-fire-and-forget-test-start]
  :end-before: [skeleton-fire-and-forget-test-end]

Publishing Broadcasts/Events as a Skeleton
------------------------------------------

Skeleton broadcasts are generated as ``<broadcastName>`` event objects with ``send(payload)``.
All connected and subscribed proxies receive the event.

.. literalinclude:: ../examples/release/skeleton_app.h
   :language: cpp
   :start-after: [service-skeleton-broadcast-start]
   :end-before: [service-skeleton-broadcast-end]

Testing Event Publishing as a Skeleton
++++++++++++++++++++++++++++++++++++++

The skeleton mock verifies that an application event is sent with the expected
payload and return status.

.. literalinclude:: ../examples/test/skeleton_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [skeleton-event-send-test-start]
  :end-before: [skeleton-event-send-test-end]

Publishing and Handling Attributes/Events as a Skeleton
-------------------------------------------------------

On the skeleton side, generated attributes expose:

* ``get()`` for local state access. Optionally overrides for generated get request handlers from proxies.

.. literalinclude:: ../examples/release/skeleton_app.h
   :language: cpp
   :start-after: [service-skeleton-attribute-get-start]
   :end-before: [service-skeleton-attribute-get-end]

* ``set(value)`` for local state access. Optionally overrides for generated set request handlers from proxies.

.. literalinclude:: ../examples/release/skeleton_app.h
   :language: cpp
   :start-after: [service-skeleton-attribute-set-start]
   :end-before: [service-skeleton-attribute-set-end]

* ``send()`` to notify subscribed proxies about the current attribute value.

.. literalinclude:: ../examples/release/skeleton_app.h
   :language: cpp
   :start-after: [service-skeleton-attribute-broadcast-start]
   :end-before: [service-skeleton-attribute-broadcast-end]

Testing Attribute Getters and Setters as a Skeleton
+++++++++++++++++++++++++++++++++++++++++++++++++++

The skeleton mock verifies that generated attribute getter and setter requests
reach the corresponding application overrides.

.. literalinclude:: ../examples/test/skeleton_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [skeleton-attribute-get-test-start]
  :end-before: [skeleton-attribute-get-test-end]

.. literalinclude:: ../examples/test/skeleton_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [skeleton-attribute-set-test-start]
  :end-before: [skeleton-attribute-set-test-end]

Testing Attribute Publishing as a Skeleton
++++++++++++++++++++++++++++++++++++++++++

The attribute send test verifies that the application publishes the expected
attribute value and handles the generated status result.

.. literalinclude:: ../examples/test/skeleton_mock_unit_test_example.cpp
  :language: cpp
  :start-after: [skeleton-attribute-send-test-start]
  :end-before: [skeleton-attribute-send-test-end]

Error Handling
--------------

The generated APIs use two error channels:

* **Immediate middleware return codes** via ``::middleware::core::HRESULT``.
* **Asynchronous method completion states** via ``::middleware::core::Future::State``.

Common ``HRESULT`` values for service APIs:

.. list-table:: Typical HRESULT values in service APIs
   :header-rows: 1
   :align: center

   * - Code
     - Meaning
     - Typical action
   * - ``Ok``
     - Operation accepted by middleware
     - Continue normal flow
   * - ``ServiceNotFound``
     - Target service instance not available
     - Verify ``InstanceId`` and startup order
   * - ``ServiceBusy``
     - Service temporarily cannot process more requests
     - Retry later or apply backoff
   * - ``QueueFull``
     - Transport queue is full
     - Retry later and review queue sizing/load
   * - ``RequestPoolDepleted``
     - No free request slots for new method calls
     - Reduce outstanding requests or increase configured capacity
   * - ``FutureAlreadyInUse`` / ``FutureNotFound``
     - Inconsistent request tracking state
     - Check callback/request lifecycle usage

Method callback completion states (``Future::State``):

.. list-table:: Method callback states
   :header-rows: 1
   :align: center

   * - State
     - Meaning
   * - ``Ready``
     - Response payload was received successfully and is available to the callback
   * - ``Timeout``
     - No response within configured timeout
   * - ``UserError``
     - Remote application reported an application-level error
   * - ``ServiceBusy``
     - Provider could not accept/process request resources
   * - ``ServiceNotFound``
     - Target provider not reachable
   * - ``SerializationError`` / ``DeserializationError``
     - Payload conversion failed
   * - ``CouldNotDeliverError``
     - Transport delivery failed

For asynchronous skeleton methods, ensure every received request is eventually completed by calling either ``respond(...)`` or the matching ``cancel...Response(...)`` API.
