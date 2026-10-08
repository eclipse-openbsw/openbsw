..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Proxy/Skeleton Base
===================

This unit provides the proxy- and skeleton-facing base types that generated
service-specific middleware bindings inherit from.

It owns initialization, request/header generation, event and attribute
distribution, and lifecycle checks for the proxy/skeleton communication API.

Class Diagram
-------------

.. uml:: proxy_skeleton_base.puml
