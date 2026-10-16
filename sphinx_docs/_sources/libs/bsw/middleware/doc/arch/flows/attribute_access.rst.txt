..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Attribute Access
================

This functionality enables proxies to read (get) and receive attribute
notifications. Attributes (also called "fields") behave like both methods
(getter/setter operations) and events (notification delivery). The skeleton
stores the attribute value and can notify client proxies when the application
emits an attribute notification.

Static View
-----------

The following software units are involved in attribute access:

* Transceiver Unit (Proxy side): Manages attribute get requests and receive-handler registration
  (see :doc:`../units/proxy_skeleton_base` for its class diagram)
* Transceiver Unit (Skeleton side): Stores attribute values and emits attribute notifications
  (see :doc:`../units/proxy_skeleton_base`)
* Request/Response Unit: Handles asynchronous getter responses
  (see :doc:`../units/request_response` for its class diagram)
* Cluster Connection Unit: Routes attribute requests and notifications between clusters
* Message Queue Unit: Delivers attribute messages between participating clusters

.. uml:: attribute_access_component.puml
   :caption: Attribute access component view

Notes:

* ``ProxyAttribute<T>`` provides a ``get(callback)`` method that sends a get
  request and keeps the callback until the getter response arrives.
* ``SkeletonAttribute<T>`` stores the attribute value.
* Getter responses use the Future mechanism (like method calls).
* Change notifications use the event broadcast mechanism.
* Attributes can have decorators based on the interface definition:

  * Default (no decorators) — setter, getter and event subscriptions allowed
  * ``readonly`` — getter and event subscriptions allowed
  * ``noSubscriptions`` — setter and getter allowed
  * ``readonly`` / ``noSubscriptions`` — getter allowed

Dynamic View
------------

The dynamic view illustrates two attribute access patterns: asynchronous get
and attribute notification.

Getter
++++++

.. uml:: attribute_access_getter_sequence.puml
   :caption: Attribute getter sequence

Change Notification
+++++++++++++++++++

.. uml:: attribute_access_change_notification_sequence.puml
   :caption: Attribute change notification sequence

Notes:

* The skeleton can update attribute values at any time using
  ``attribute.set(newValue)``.
* The application explicitly sends attribute notifications through the
  generated attribute ``send()`` operation. Updating the local value alone
  does not notify subscribed proxies.
* Proxies must call ``attribute.setReceiveHandler(callback)`` to receive
  attribute notifications.
* The proxy callback is invoked when the notification message is processed.

