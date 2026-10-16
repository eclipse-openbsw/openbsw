..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Future Objects Changelog
========================

Version 26KW39
--------------

Rename generated skeleton respond and attribute accessor methods
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

- **Duplicate definitions fixed**: generated ``SkeletonBase`` subclasses previously reused a single ``respond(...)`` overload set for every method and attribute, which could collide when a service had multiple operations with matching signatures.
- **Method responses**: ``respond(...)`` is now generated per method as ``respond<MethodName>(...)``.
- **Attribute responses**: attribute getter/setter responses are now ``respondGet<AttributeName>Attribute(...)`` and ``respondSet<AttributeName>Attribute(...)``.
- **Attribute accessors renamed**: ``get_<AttributeName>Attribute(...)`` and ``set_<AttributeName>Attribute(...)`` are now ``get<AttributeName>Attribute(...)`` and ``set<AttributeName>Attribute(...)`` (underscore removed).
- **Cancel APIs renamed**: ``cancelGet_<AttributeName>AttributeResponse(...)`` and ``cancelSet_<AttributeName>AttributeResponse(...)`` are now ``cancelGet<AttributeName>AttributeResponse(...)`` and ``cancelSet<AttributeName>AttributeResponse(...)``.
- **New setter response signature**: for ``setAsMethod`` attributes, ``respondSet<AttributeName>Attribute(response, bool result, handleResponseFailure = true)`` now responds with a success flag instead of echoing back the attribute value.

Version 26KW26
--------------

Return method and getter payloads by const reference
++++++++++++++++++++++++++++++++++++++++++++++++++++

Proxy Methods and Attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- **Callback payload type**: Request/response method and getter callbacks now receive ``etl::expected<etl::reference_wrapper<PayloadType const>, Future::State>`` instead of ``etl::expected<PayloadType, Future::State>``. Access the payload via ``output.value().get()`` instead of ``output.value()``.
- **New result aliases**: The generated proxy now exposes a ``<MethodName>Result`` alias, and ``ProxyAttributeBase`` / ``ProxyAttribute`` expose ``GetterResult`` / ``RequestResult`` aliases for these callback result types.


Proxy Attributes changes
~~~~~~~~~~~~~~~~~~~~~~~~

- **How to cancel a request**: ``invalidateFuture(Future future)`` is now called ``cancelGetterRequest(uint16_t reqId)`` or ``cancelSetterRequest(uint16_t reqId)`` and takes a ``uint16_t``.
- **How to answer a request**: `setResult(Message msg)` is now called `answerGetterRequest(Message msg)` or `answerSetterRequest(Message msg)`.
- **Deleted Future Objects**: ``Future`` objects are no longer the application's responsibility.
- **Changes to Get Method**: ``get(Callback& callback)`` now returns ``etl::expected<uint16_t, HRESULT>`` and takes a ``GetterPolicy::Callback`` as an input.
- **Changes to Set Method**: ``set(ArgType& payload, Callback& callback)`` now returns ``etl::expected<uint16_t, HRESULT>`` and additionally to the payload takes a callback.
- **Changes to Set Method (Fire And Forget)**: When the set method is Fire&Forget, ``set(ArgType& payload)`` returned ``uint16_t`` will always be ``INVALID_REQUEST_ID`` and takes no callback.

Proxy Methods
~~~~~~~~~~~~~

- **How to cancel a request**: ``invalidateFuture(Future future)`` no longer exists. A ``cancelRequest(uint16_t reqId)`` needs to be called now with the appropriate ``uint16_t``.
- **How to answer a request**: `setResult(Message msg)` has been replaced with `answerRequest(Message msg)`. Inside, this calls what was called `releaseRequestId(msg)` and is now called `futureMatchingRequestId(msg)`.
- **To call a method**: The method call no longer takes a future object, but a callback. The return type is now an ``etl::expected<uint16_t, HRESULT>``.

Generator Changes
~~~~~~~~~~~~~~~~~

<InterfaceName>Proxy.h
~~~~~~~~~~~~~~~~~~~~~~

- **Attribute Classes**: No aliases created, only the constructor needs to be created.

<InterfaceName>Proxy.cpp
~~~~~~~~~~~~~~~~~~~~~~~~

- ***_releaseRequestId(...) methods**: These have been removed and, instead, on the `OnNewMessageReceived` method, the dispatcher methods are directly called.
- **Method Internal Logic**: A lot of changes of the internal logic in the methods' body have been made, such as return statements and extra checks. Refer to the generated service proxy when adapting the generator.

<InterfaceName>Skeleton.h
~~~~~~~~~~~~~~~~~~~~~~~~~

- **CancelResponse**: A method for each method is now created with signature `cancel<MethodName>Response(SkeletonResponseInfo response)`.
